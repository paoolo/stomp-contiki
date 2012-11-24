#include "stomp-global.h"

#include "stomp-network.h"
#include "stomp-tools.h"
#include "stomp-queue.h"

#include "uip-debug.h"

#include <stdio.h>

struct stomp_network_state network_state;

struct stomp_queue network_send_queue;

uip_ipaddr_t stomp_network_addr;
int stomp_network_port = 61613;

#if UIP_CONF_IPV6 > 0
int stomp_network_addr_num[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
// int stomp_network_addr_num[] = {0xaaaa, 0, 0, 0, 0x280, 0xe102, 0, 0x4874};
// int stomp_network_addr_num[] = {0xaaaa, 0, 0, 0, 0, 0, 0, 1};
#else
uint8_t stomp_network_addr_num[] = {10, 1, 1, 100};
#endif

PROCESS(stomp_network_process, "STOMP network process");
PROCESS(stomp_network_send_process, "STOMP network send process");

static void
__connect(uip_ipaddr_t *addr, int port) {
#ifdef STOMP_NETWORK_TRACE
    PRINTA("__connect: connecting...\n");
#endif

#ifdef WITH_UDP
#ifdef STOMP_NETWORK_TRACE
    PRINTA("__connect: UDP\n");
#endif
    network_state.conn = udp_new(addr, UIP_HTONS(port), &network_state);
#else
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_connect: TCP\n");
#endif
    network_state.conn = tcp_connect(addr, UIP_HTONS(port), &network_state);
#endif

    if (network_state.conn == NULL) {
        return;
    }

#ifdef WITH_UDP
    udp_bind(network_state.conn, UIP_HTONS(port + 1));
#ifdef STOMP_NETWORK_TRACE
    PRINTA("__connect: binding...\n");
#endif
#endif

    network_state.addr = addr;
    network_state.port = port;
    network_state.flags = 0;
    network_state.buf = NULL;
    network_state.len = 0;
    network_state.off = 0;
    network_state.sentlen = 0;
}

static void
__senddata() {
    if (network_state.buf == NULL) {
#ifdef STOMP_NETWORK_TRACE
        PRINTA("__senddata: nothing to send, stop.\n");
#endif
        return;
    }
#ifndef WITH_UDP
    if (network_state.len > uip_mss()) {
        network_state.sentlen = uip_mss();
    } else {
        network_state.sentlen = network_state.len;
    }
#endif

#ifdef WITH_UDP
    uip_udp_packet_sendto(network_state.conn, network_state.buf, network_state.len, network_state.addr, UIP_HTONS(network_state.port));
    DELETE(network_state.buf);
#else
    uip_send(network_state.buf + network_state.off, network_state.sentlen);
#endif
}

#ifndef WITH_UDP

static void
__acked() {
    network_state.len -= network_state.sentlen;
    if (network_state.len == 0) {
        DELETE(network_state.buf);
        stomp_network_sent(network_state);
    } else {
        network_state.off += network_state.sentlen;
    }
    network_state.sentlen = 0;
}
#endif

PROCESS_THREAD(stomp_network_send_process, ev, data) {
    static struct etimer et;

    PROCESS_BEGIN();
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_send_process: start.\n");
#endif

#ifdef WITH_UDP    
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_send_process: waiting for UDP connection.\n");
#endif
    etimer_set(&et, CLOCK_CONF_SECOND * 15);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
#endif

    etimer_set(&et, CLOCK_CONF_SECOND * 1);
    while (1) {
        PROCESS_WAIT_EVENT();
        if (etimer_expired(&et)) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_send_process: timer expired.\n");
#endif
        }
        if (network_state.buf == NULL) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_send_process: buffer is free.\n");
#endif
            network_state.buf = stomp_queue_get_buf(&network_send_queue);
            if (network_state.buf != NULL) {
#ifdef STOMP_NETWORK_TRACE
                PRINTA("stomp_network_send_process: there is something to send.\n");
#endif
                network_state.len = stomp_queue_get_len(&network_send_queue);
                stomp_queue_remove(&network_send_queue);
                process_post(&stomp_network_process, PROCESS_EVENT_CONTINUE, NULL);
            } else {
                network_state.len = 0;
            }
            network_state.sentlen = 0;
        }
#ifdef STOMP_NETWORK_TRACE
        else {
            PRINTA("stomp_network_send_process: there are some data to send.\n");
        }
#endif
        etimer_restart(&et);
    }

#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_send_process: stop.\n");
#endif
    PROCESS_END();
}

PROCESS_THREAD(stomp_network_process, ev, data) {

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&stomp_network_addr, stomp_network_addr_num[0], stomp_network_addr_num[1], stomp_network_addr_num[2], stomp_network_addr_num[3], stomp_network_addr_num[4], stomp_network_addr_num[5], stomp_network_addr_num[6], stomp_network_addr_num[7]);
#else
    uip_ipaddr(&stomp_network_addr, stomp_network_addr_num[0], stomp_network_addr_num[1], stomp_network_addr_num[2], stomp_network_addr_num[3]);
#endif

    PROCESS_BEGIN();
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_process: start.\n");
#endif

    __connect(&stomp_network_addr, stomp_network_port);

    while (1) {
#ifdef STOMP_NETWORK_TRACE
        PRINTA("stomp_network_process: wait for any event.\n");
#endif
        PROCESS_WAIT_EVENT();
#ifdef STOMP_NETWORK_TRACE
        PRINTA("stomp_network_process: any event.\n");
#endif
#ifndef WITH_UDP
        if (uip_connected()) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: connected.\n");
#endif
            network_state.flags = 0;
            stomp_network_connected();
            __senddata();
            continue;
        }
        if (uip_closed()) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: closed.\n");
#endif
            stomp_network_closed();
        }
        if (uip_aborted()) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: aborted.\n");
#endif
            stomp_network_aborted();
        }
        if (uip_timedout()) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: timedout.\n");
#endif
            stomp_network_timedout();
        }
        if (network_state.flags & STOMP_FLAG_DISCONNECT) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: closing.\n");
#endif
            uip_close();
            continue;
        }
        if (network_state.flags & STOMP_FLAG_ABORT) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: aborting.\n");
#endif
            uip_abort();
            continue;
        }
        if (uip_acked()) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: acked.\n");
#endif
            __acked();
        }
#endif
        if (uip_newdata()) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: new data.\n");
#endif
            stomp_network_received((char*) uip_appdata, uip_datalen());
        }
#ifndef WITH_UDP
        if (uip_rexmit() || uip_newdata() || uip_acked()) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: rexmit || new data || acked.\n");
#endif
            __senddata();

        } else if (uip_poll()) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: poll.\n");
#endif
            __senddata();
        }
#endif
        if (network_state.buf != NULL) {
#ifdef STOMP_NETWORK_TRACE
            PRINTA("stomp_network_process: send data\n");
#endif
            __senddata();
        }
    }

#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_process: stop.\n");
#endif
    PROCESS_END();
}

unsigned char
stomp_network_send(char *buf, int len) {
    stomp_queue_add(buf, len, &network_send_queue);
    process_post(&stomp_network_send_process, PROCESS_EVENT_CONTINUE, NULL);
    return 0;
}

#ifndef WITH_UDP

unsigned char
stomp_network_close() {
    network_state.flags = STOMP_FLAG_DISCONNECT;
    if (network_state.buf != NULL) {
#ifdef STOMP_NETWORK_TRACE
        PRINTA("stomp_network_close: there was something to send.\n");
#endif
        return 1;
    }
    return 0;
}
#endif

#ifndef WITH_UDP

unsigned char
stomp_network_abort() {
    network_state.flags = STOMP_FLAG_ABORT;
    if (network_state.buf != NULL) {
#ifdef STOMP_NETWORK_TRACE
        PRINTA("stomp_network_abort: there was something to send.\n");
#endif
        return 1;
    }
    return 0;
}
#endif