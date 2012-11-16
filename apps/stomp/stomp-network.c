#include "stomp-network.h"

#include "contiki.h"
#include "contiki-net.h"

#include "stomp.h"
#include "stomp-tools.h"
#include "stomp-frame.h"
#include "stomp-network.h"
#include "stomp-strings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct stomp_network_state network_state;

uip_ipaddr_t stomp_network_addr;
int stomp_network_port = 61613;

#if UIP_CONF_IPV6 > 0
uint16_t addr_num[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
#else
uint8_t addr_num[] = {10, 1, 1, 100};
#endif

PROCESS(stomp_network_process, "STOMP network process");
AUTOSTART_PROCESSES(&stomp_network_process);

static void
__senddata() {
    printf("__senddata: start.\n");

    if (network_state.buf == NULL) {
        return;
    }
    if (network_state.len > uip_mss()) {
        network_state.sentlen = uip_mss();
    } else {
        network_state.sentlen = network_state.len;
    }

#ifdef WITH_UDP
    uip_udp_packet_sendto(network_state.conn, network_state.buf + network_state.off, network_state.len, network_state.addr, UIP_HTONS(network_state.port));
#else
    uip_send(network_state.buf + network_state.off, network_state.sentlen);
#endif

    printf("__senddata: stop.\n");
}

static void
__acked() {
    printf("__acked: start.\n");

    network_state.len -= network_state.sentlen;
    if (network_state.len == 0) {
        DELETE(network_state.buf);
        stomp_network_sent(network_state);
    } else {
        network_state.off += network_state.sentlen;
    }
    network_state.sentlen = 0;

    printf("__acked: stop.\n");
}

PROCESS_THREAD(stomp_network_process, ev, data) {

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&stomp_network_addr, addr_num[0], addr_num[1], addr_num[2], addr_num[3], addr_num[4], addr_num[5], addr_num[6], addr_num[7]);
#else
    uip_ipaddr(&stomp_network_addr, addr_num[0], addr_num[1], addr_num[2], addr_num[3]);
#endif

    PROCESS_BEGIN();
    printf("stompc_process: start.\n");

    stomp_network_connect(&stomp_network_addr, stomp_network_port);
    
    while (1) {
        PROCESS_WAIT_EVENT();
        printf("stompc_process: any event.\n");
        printf("stomp_network_process: start.\n");
        if (uip_connected()) {
            printf("stomp_network_process: connected.\n");
            network_state.flags = 0;
            stomp_network_connected();
            __senddata();
            continue;
        }
        if (uip_closed()) {
            printf("stomp_network_process: closed.\n");
            stomp_network_closed();
        }
        if (uip_aborted()) {
            printf("stomp_network_process: aborted.\n");
            stomp_network_aborted();
        }
        if (uip_timedout()) {
            printf("stomp_network_process: timedout.\n");
            stomp_network_timedout();
        }
        if (network_state.flags & STOMP_FLAG_DISCONNECT) {
            printf("stomp_network_process: closing.\n");
            uip_close();
            continue;
        }
        if (network_state.flags & STOMP_FLAG_ABORT) {
            printf("stomp_network_process: aborting.\n");
            uip_abort();
            continue;
        }
        if (uip_acked()) {
            printf("stomp_network_process: acked.\n");
            __acked();
        }
        if (uip_newdata()) {
            printf("stomp_network_process: new data.\n");
            stomp_network_received((char*) uip_appdata, uip_datalen());
        }
        if (uip_rexmit() || uip_newdata() || uip_acked()) {
            printf("stomp_network_process: rexmit || new data || acked.\n");
            __senddata();
        } else if (uip_poll()) {
            printf("stomp_network_process: poll.\n");
            __senddata();
        }
    }

    printf("stomp_network_process: stop.\n");
    PROCESS_END();
}

void
stomp_network_connect(uip_ipaddr_t *addr, uint16_t port) {
    printf("stomp_network_connect: start.\n");

    printf("stomp_network_connect: connecting...\n");

#ifdef WITH_UDP
    printf("stomp_network_connect: UDP connect\n");
    network_state.conn = udp_new(addr, UIP_HTONS(port), &network_state);
#else
    printf("stomp_network_connect: TCP connect\n");
    network_state.conn = tcp_connect(addr, UIP_HTONS(port), &network_state);
#endif

    if (network_state.conn == NULL) {
        printf("stomp_network_connect: not connected.\n");
        return;
    }

#ifdef WITH_UDP
    udp_bind(network_state.conn, UIP_HTONS(port + 1));
    printf("stomp_network_connect: binding...\n");
#endif

    network_state.addr = addr;
    network_state.port = port;
    network_state.flags = 0;
    network_state.buf = NULL;
    network_state.len = 0;
    network_state.off = 0;
    network_state.sentlen = 0;

    printf("stomp_network_connect: stop.\n");
}

unsigned char
stomp_network_send(char *buf, uint16_t len) {
    printf("stomp_network_send: start.\n");

    if (network_state.buf != NULL) {
        printf("stomp_network_send: there is something still sending.\n");
        printf("stomp_network_send: stop.\n");
        return 1;
    }

    network_state.buf = buf;
    network_state.len = len;
    network_state.sentlen = 0;

    printf("stomp_network_send: stop.\n");
    return 0;
}

unsigned char
stomp_network_disconnect() {
    printf("stomp_network_close: start.\n");

    network_state.flags = STOMP_FLAG_DISCONNECT;
    if (network_state.buf != NULL) {
        printf("stomp_network_close: there was something to send.\n");

        printf("stomp_network_close: stop.\n");
        return 1;
    }

    printf("stomp_network_close: stop.\n");
    return 0;
}

unsigned char
stomp_network_abort() {
    printf("stomp_network_abort: start.\n");

    network_state.flags = STOMP_FLAG_ABORT;
    if (network_state.buf != NULL) {
        printf("stomp_network_abort: there was something to send.\n");

        printf("stomp_network_abort: stop.\n");
        return 1;
    }

    printf("stomp_network_abort: stop.\n");
    return 0;
}
