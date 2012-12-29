#include "stomp-network.h"
#include "stomp-tools.h"
#include "stomp-queue.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"

#include <stdio.h>

struct stomp_network_state network_state;

struct stomp_queue network_send_queue;

uip_ipaddr_t ipaddr;
int port = 61613;

#if UIP_CONF_IPV6 > 0
int addr[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
#else
int addr[] = {10, 1, 1, 100};
#endif

PROCESS(stomp_network_process, "STOMP network process");
PROCESS(stomp_network_send_process, "STOMP network send process");

static void
__sent() {
    stomp_network_sent(network_state.buf, network_state.len);
    DELETE(network_state.buf);
}

static void
__send() {
    if (network_state.buf == NULL) {
        return;
    }
#ifndef WITH_UDP
    if (network_state.len > uip_mss()) {
        network_state.sentlen = uip_mss();
    } else {
        network_state.sentlen = state.len;
    }
#endif
#ifdef WITH_UDP
    uip_udp_packet_sendto(network_state.conn, network_state.buf + network_state.off,
            network_state.len, network_state.addr, UIP_HTONS(network_state.port));
#else
    uip_send(network_state.buf + network_state.off, network_state.sentlen);
#endif
}

#ifndef WITH_UDP

static void
__acked() {
    network_state.len -= network_state.sentlen;
    if (network_state.len == 0) {
        __sent();
    } else {
        network_state.off += network_state.sentlen;
    }
    network_state.sentlen = 0;
}
#endif

PROCESS_THREAD(stomp_network_send_process, ev, data) {
    static struct etimer et;
    PROCESS_BEGIN();
#ifdef WITH_UDP
    etimer_set(&et, CLOCK_CONF_SECOND * 15);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
#endif
    etimer_set(&et, CLOCK_CONF_SECOND * 1);
    while (1) {
        PROCESS_WAIT_EVENT();
        if (network_state.buf == NULL) {
            network_state.buf = stomp_queue_get_buf(&network_send_queue);
            if (network_state.buf != NULL) {
                network_state.len = stomp_queue_get_len(&network_send_queue);
                stomp_queue_remove(&network_send_queue);
                process_post(&stomp_network_process, PROCESS_EVENT_CONTINUE, NULL);
            } else {
                network_state.len = 0;
            }
            network_state.sentlen = 0;
        }
        etimer_restart(&et);
    }
    PROCESS_END();
}

PROCESS_THREAD(stomp_network_process, ev, data) {
#ifdef WITH_UDP
    static struct etimer et;
#endif

    PROCESS_BEGIN();

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&ipaddr, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
#else
    uip_ipaddr(&ipaddr, addr[0], addr[1], addr[2], addr[3]);
#endif

    stomp_network_connect(&ipaddr, port);
#ifdef WITH_UDP
    etimer_set(&et, CLOCK_CONF_SECOND * 15);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    stomp_network_connected();
#endif

    while (1) {
        PROCESS_WAIT_EVENT();
#ifdef WITH_UDP
        if (ev == tcpip_event) {
            if (uip_newdata()) {
                if (uip_datalen() == 2) {
                    PRINTA("Sent.\n");
                    __sent();
                } else {
                    char *str = (char*) uip_appdata;
                    str[uip_datalen()] = '\0';
                    stomp_network_received(str, uip_datalen());
                }
            }
        } else if (network_state.buf != NULL) {
            __send();
        }
#else
        if (uip_connected()) {
            network_state.flags = 0;
            stomp_network_connected();
            __send();
            continue;
        }
        if (uip_closed()) {
            stomp_network_closed();
        }
        if (uip_aborted()) {
            stomp_network_aborted();
        }
        if (uip_timedout()) {
            stomp_network_timedout();
        }
        if (network_state.flags & STOMP_FLAG_DISCONNECT) {
            uip_close();
            continue;
        }
        if (network_state.flags & STOMP_FLAG_ABORT) {
            uip_abort();
            continue;
        }
        if (uip_acked()) {
            __acked();
        }
        if (uip_newdata()) {
            stomp_network_received((char*) uip_appdata, uip_datalen());
        }
        if (uip_rexmit() || uip_newdata() || uip_acked()) {
            __send();

        } else if (uip_poll()) {
            __send();
        }
#endif
    }
    PROCESS_END();
}

void
stomp_network_connect(uip_ipaddr_t *addr, int port) {
#ifdef WITH_UDP
    network_state.conn = udp_new(addr, UIP_HTONS(port), &network_state);
#else
    network_state.conn = tcp_connect(addr, UIP_HTONS(port), &network_state);
#endif
    if (network_state.conn == NULL) {
        return;
    }
#ifdef WITH_UDP
    udp_bind(network_state.conn, UIP_HTONS(port + 1));
#endif
    network_state.addr = addr;
    network_state.port = port;
    network_state.flags = 0;
    network_state.buf = NULL;
    network_state.len = 0;
    network_state.off = 0;
    network_state.sentlen = 0;
}

#ifndef WITH_UDP

void
stomp_network_timedout() {
    PRINTA("Timedout.\n");
}

void
stomp_network_abort() {
    network_state.flags = STOMP_FLAG_ABORT;
}

void
stomp_network_aborted() {
    PRINTA("Aborted.\n");
}

void
stomp_network_close() {
    network_state.flags = STOMP_FLAG_DISCONNECT;
}

void
stomp_network_closed() {
    PRINTA("Closed.\n");
}
#endif

void
stomp_network_send(char *buf, int len) {
    stomp_queue_add(buf, len, &network_send_queue);
    process_post(&stomp_network_send_process, PROCESS_EVENT_CONTINUE, NULL);
}