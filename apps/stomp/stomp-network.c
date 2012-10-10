#include "stomp-network.h"

#include "contiki-net.h"

#include "stompc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct stomp_state*
stomp_network_connect(struct stomp_state *state, uip_ipaddr_t *address, uint16_t port)
{
    state->network_state.conn = tcp_connect(address, UIP_HTONS(port), state);
    if (state->network_state.conn == NULL) {
        printf("Cannot create new TCP connection - Out of memory error.");
        return NULL;
    }

    state->network_state.address = address;
    state->network_state.port = port;

    state->network_state.flags = 0;
    state->network_state.buffer = NULL;
    state->network_state.bufferlen = 0;
    state->network_state.sentlen = 0;

    return state;
}

unsigned char
stomp_network_send(struct stomp_state *state, char *buffer, uint16_t len)
{
    if (state->network_state.buffer != NULL) {
        printf("Cannot send data - Buffer is not empty.");
        return 1;
    }
    state->network_state.buffer = buffer;
    state->network_state.bufferlen = len;
    state->network_state.sentlen = 0;
    return 0;
}

static void
stomp_network_senddata(struct stomp_state *state)
{
    if (state->network_state.buffer == NULL) {
        uip_send(state->network_state.buffer, 0);
        return;
    }
    if (state->network_state.bufferlen > uip_mss()) {
        state->network_state.sentlen = uip_mss();
    } else {
        state->network_state.sentlen = state->network_state.bufferlen;
    }
    uip_send(state->network_state.buffer, state->network_state.sentlen);
}

unsigned char
stomp_network_close(struct stomp_state *state)
{
    state->network_state.flags = STOMP_FLAG_CLOSE;
    if (state->network_state.buffer != NULL) {
        printf("Cannot close - Buffer is not empty.");
        return 1;
    }
    return 0;
}

unsigned char
stomp_network_abort(struct stomp_state *state)
{
    state->network_state.flags = STOMP_FLAG_ABORT;
    if (state->network_state.buffer != NULL) {
        printf("Cannot abort - Buffer is not empty.");
        return 1;
    }
    return 0;
}

static void
stomp_network_acked(struct stomp_state *state)
{
    state->network_state.bufferlen -= state->network_state.sentlen;
    if (state->network_state.bufferlen == 0) {
        state->network_state.buffer = NULL;
        stomp_network_sent(state);
    } else {
        state->network_state.buffer += state->network_state.sentlen;
    }
    state->network_state.sentlen = 0;
}

void
stomp_network_app(void *s)
{
    struct stomp_state *state = (struct stomp_state*)s;
    
    if (uip_connected()) {
        state->network_state.flags = 0;
        stomp_network_connected(state);
        return;
    }
    
    if (uip_closed()) {
        stomp_network_closed(state);
    }
    if (uip_aborted()) {
        stomp_network_aborted(state);
    }
    if (uip_timedout()) {
        stomp_network_timedout(state);
    }
    if (state->network_state.flags & STOMP_FLAG_CLOSE) {
        uip_close();
        return;
    }
    if (state->network_state.flags & STOMP_FLAG_ABORT) {
        uip_abort();
        return;
    }
    if (uip_acked()) {
        stomp_network_acked(state);
    }
    if (uip_newdata()) {
        stomp_network_received(state, (char*)uip_appdata, uip_datalen());
    }
    if (uip_rexmit() || uip_newdata() || uip_acked()) {
        stomp_network_senddata(state);
    } else if (uip_poll()) {
        stomp_network_senddata(state);
    }
}