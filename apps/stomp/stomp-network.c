#include "stomp-network.h"

#include "contiki-net.h"

#include "stompc.h"
#include "stomp-frame.h"
#include "stomp-tools.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct stomp_network_state*
stomp_network_connect(struct stomp_network_state *state, uip_ipaddr_t *addr, uint16_t port) {
    printf("stomp_network_connect: start.\n");

    printf("stomp_network_connect: connecting...\n");

#ifdef WITH_UDP
    printf("stomp_network_connect: UDP connect\n");
    state->conn = udp_new(addr, UIP_HTONS(port), state);
#else
    printf("stomp_network_connect: TCP connect\n");
    state->conn = tcp_connect(addr, UIP_HTONS(port), state);
#endif

    if (state->conn == NULL) {
        printf("stomp_network_connect: not connected.\n");
        return NULL;
    }

#ifdef WITH_UDP
    udp_bind(state->conn, UIP_HTONS(port + 1));
    printf("stomp_network_connect: binding...\n");
#endif

    state->addr = addr;
    state->port = port;

    state->flags = 0;

    state->outputbuf_len = 0;
    state->outputbuf_sentlen = 0;
    memset(state->outputbuf, 0, STOMP_OUTPUTBUF_SIZE);

    state->inputbuf_len = 0;
    state->inputbuf_sentlen = 0;
    memset(state->inputbuf, 0, STOMP_INPUTBUF_SIZE);

    printf("stomp_network_connect: stop.\n");

    return state;
}

unsigned char
stomp_network_send(struct stomp_network_state *state, const char *buf, uint16_t len) {
    printf("stomp_network_send: start.\n");

    if (state->outputbuf_len > 0) {
        printf("stomp_network_send: there is something still sending.\n");

        printf("stomp_network_send: stop.\n");
        return 1;
    }
    memcpy(state->outputbuf, buf, len);
    state->outputbuf_len = len;
    state->outputbuf_sentlen = 0;

    printf("stomp_network_send: stop.\n");
    return 0;
}

static void
__udp_senddata(struct stomp_network_state *state) {
    printf("__udp_senddata: start.\n");

    uip_udp_packet_sendto(state->conn, state->outputbuf, state->outputbuf_len, state->addr, UIP_HTONS(state->port));

    printf("__udp_senddata: stop.\n");
}

static void
__tcp_senddata(struct stomp_network_state *state) {
    printf("__tcp_send: start.\n");

    if (state->outputbuf_len == 0) {
        uip_send(state->outputbuf, 0);
        return;
    }
    if (state->outputbuf_len > uip_mss()) {
        state->outputbuf_sentlen = uip_mss();
    } else {
        state->outputbuf_sentlen = state->outputbuf_len;
    }
    uip_send(state->outputbuf, state->outputbuf_sentlen);

    printf("__tcp_send: stop.\n");
}

static void
__senddata(struct stomp_network_state *state) {
    printf("__senddata: start.\n");

#ifdef WITH_UDP
    __udp_senddata(state);
#else
    __tcp_senddata(state);
#endif

    printf("__senddata: stop.\n");
}

unsigned char
stomp_network_close(struct stomp_network_state *state) {
    printf("stomp_network_close: start.\n");

    state->flags = STOMP_FLAG_CLOSE;
    if (state->outputbuf_len > 0) {
        printf("stomp_network_close: there was something to send.\n");

        printf("stomp_network_close: stop.\n");
        return 1;
    }

    printf("stomp_network_close: stop.\n");
    return 0;
}

unsigned char
stomp_network_abort(struct stomp_network_state *state) {
    printf("stomp_network_abort: start.\n");

    state->flags = STOMP_FLAG_ABORT;
    if (state->outputbuf_len > 0) {
        printf("stomp_network_abort: there was something to send.\n");

        printf("stomp_network_abort: stop.\n");
        return 1;
    }

    printf("stomp_network_abort: stop.\n");
    return 0;
}

static void
__acked(struct stomp_network_state *state) {
    char temp[STOMP_OUTPUTBUF_SIZE];

    printf("__acked: start.\n");

    state->outputbuf_len -= state->outputbuf_sentlen;
    if (state->outputbuf_len == 0) {
        memset(state->outputbuf, 0, STOMP_OUTPUTBUF_SIZE);
        stomp_network_sent(state);
    } else {
        memcpy(temp, state->outputbuf + state->outputbuf_sentlen, state->outputbuf_len);
        memset(state->outputbuf, 0, STOMP_OUTPUTBUF_SIZE);
        memcpy(state->outputbuf, temp, state->outputbuf_len);
    }
    state->outputbuf_sentlen = 0;

    printf("__acked: stop.\n");
}

void
stomp_network_app(void *s) {
    struct stomp_network_state *state = (struct stomp_network_state*) s;

    printf("stomp_network_app: start.\n");

    if (uip_connected()) {
        printf("stomp_network_app: connected.\n");
        state->flags = 0;
        stomp_network_connected(state);
        __senddata(state);
        return;
    }

    if (uip_closed()) {
        printf("stomp_network_app: closed.\n");
        stomp_network_closed(state);
    }
    if (uip_aborted()) {
        printf("stomp_network_app: aborted.\n");
        stomp_network_aborted(state);
    }
    if (uip_timedout()) {
        printf("stomp_network_app: timedout.\n");
        stomp_network_timedout(state);
    }
    if (state->flags & STOMP_FLAG_CLOSE) {
        printf("stomp_network_app: closing.\n");
        uip_close();
        return;
    }
    if (state->flags & STOMP_FLAG_ABORT) {
        printf("stomp_network_app: aborting.\n");
        uip_abort();
        return;
    }
    if (uip_acked()) {
        printf("stomp_network_app: acked.\n");
        __acked(state);
    }
    if (uip_newdata()) {
        printf("stomp_network_app: new data.\n");
        stomp_network_received(state, (char*) uip_appdata, uip_datalen());
    }
    if (uip_rexmit() || uip_newdata() || uip_acked()) {
        printf("stomp_network_app: rexmit || new data || acked.\n");
        __senddata(state);

    } else if (uip_poll()) {
        printf("stomp_network_app: poll.\n");
        __senddata(state);
    }

    printf("stomp_network_app: stop.\n");
}