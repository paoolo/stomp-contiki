#include "ultra-simple-stomp-network.h"

#include "stomp-tools.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"

#include <string.h>

struct ultra_simple_stomp state;

static void
__sent() {
    DELETE(state.buf);
    stomp_net_sent();
}

static void
__send() {
    if (state.buf == NULL) {
        return;
    }
#ifndef WITH_UDP
    if (state.len > uip_mss()) {
        state.sentlen = uip_mss();
    } else {
        state.sentlen = state.len;
    }
#endif
#ifdef WITH_UDP
    uip_udp_packet_sendto(state.conn, state.buf + state.off, state.len, state.addr, UIP_HTONS(state.port));
    __sent();
#else
    uip_send(state.buf + state.off, state.sentlen);
#endif
}

#ifndef WITH_UDP

static void
__acked() {
    state.len -= state.sentlen;
    if (state.len == 0) {
        __sent();
    } else {
        state.off += state.sentlen;
    }
    state.sentlen = 0;
}
#endif

int
addr[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};

uip_ipaddr_t
ipaddr;

int
port = 61613;

PROCESS(ultra_simple_stomp_network_process, "ultra-simple-STOMP network process");

PROCESS_THREAD(ultra_simple_stomp_network_process, ev, data) {
#ifdef WITH_UDP
    static struct etimer et;
#endif

    PROCESS_BEGIN();

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&ipaddr, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
#else
    uip_ipaddr(&ipaddr, addr[0], addr[1], addr[2], addr[3]);
#endif

    stomp_net_connect(&ipaddr, port);
#ifdef WITH_UDP
    etimer_set(&et, CLOCK_CONF_SECOND * 15);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    stomp_net_connected();
#endif

    while (1) {
        PROCESS_WAIT_EVENT();
#ifdef WITH_UDP
        if (ev == tcpip_event) {
            if (uip_newdata()) {
                stomp_net_received((char*) uip_appdata, uip_datalen());
            }
        } else {
            if (state.buf != NULL) {
                __send();
            }
        }
#else
        if (uip_connected()) {
            state.flags = 0;
            stomp_net_connected();
            __send();
            continue;
        }
        if (uip_closed()) {
            stomp_net_closed();
        }
        if (uip_aborted()) {
            stomp_net_aborted();
        }
        if (uip_timedout()) {
            stomp_net_timedout();
        }
        if (state.flags & ULTRA_SIMPLE_STOMP_FLAG_DISCONNECT) {
            uip_close();
            continue;
        }
        if (state.flags & ULTRA_SIMPLE_STOMP_FLAG_ABORT) {
            uip_abort();
            continue;
        }
        if (uip_acked()) {
            __acked();
        }
        if (uip_newdata()) {
            stomp_net_received((char*) uip_appdata, uip_datalen());
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
stomp_net_connect(uip_ipaddr_t *ipaddr, int port) {
#ifdef WITH_UDP
    state.conn = udp_new(ipaddr, UIP_HTONS(port), &state);
#else
    state.conn = tcp_connect(ipaddr, UIP_HTONS(port), &state);
#endif
    if (state.conn == NULL) {
        return;
    }
#ifdef WITH_UDP
    udp_bind(state.conn, UIP_HTONS(port + 1));
#endif
    state.addr = ipaddr;
    state.port = port;
    state.flags = 0;
    state.len = 0;
    state.buf = NULL;
    state.off = 0;
    state.sentlen = 0;
}

#ifndef WITH_UDP

void
stomp_net_timedout() {
    PRINTA("Timedout.\n");
}

void
stomp_net_abort() {
    state.flags = ULTRA_SIMPLE_STOMP_FLAG_ABORT;
}

void
stomp_net_aborted() {
    PRINTA("Aborted.\n");
}

void
stomp_net_close() {
    state.flags = ULTRA_SIMPLE_STOMP_FLAG_DISCONNECT;
}

void
stomp_net_closed() {
    PRINTA("Closed.\n");
}
#endif

void
stomp_net_send(struct process *proc, char *buf, int len) {
    state.buf = buf;
    state.len = len;
#ifdef WITH_UDP
    process_post(&ultra_simple_stomp_network_process, PROCESS_EVENT_CONTINUE, NULL);
#endif
}