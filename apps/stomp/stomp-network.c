#include "contiki-net.h"

#include "stomp-network.h"

struct stomp_network_state *
stomp_network_connect(struct stomp_network_state *state, uip_ipaddr_t *address, uint16_t port)
{
    state->conn = tcp_connect(address, UIP_HTONS(port), state);
    if (state->conn == NULL) {
        return NULL;
    }

    state->address = address;
    state->port = port;

    state->flags = 0;
    state->buffer = NULL;
    state->bufferlen = 0;
    state->sentlen = 0;

    return state;
}

unsigned char
stomp_network_send(struct stomp_network_state *state, char *buffer, uint16_t len)
{
    if (state->buffer != NULL) {
        return 1;
    }
    state->buffer = buffer;
    state->bufferlen = len;
    state->sentlen = 0;
    return 0;
}

static void
stomp_network_senddata(struct stomp_network_state *state)
{
    if (state->buffer == NULL) {
        uip_send(state->buffer, 0);
        return;
    }
    if (state->bufferlen > uip_mss()) {
        state->sentlen = uip_mss();
    } else {
        state->sentlen = state->bufferlen;
    }
    uip_send(state->buffer, state->sentlen);
}

unsigned char
stomp_network_close(struct stomp_network_state *state)
{
    state->flags = FLAG_CLOSE;
    if (state->buffer != NULL) {
        return 1;
    }
    return 0;
}

unsigned char
stomp_network_abort(struct stomp_network_state *state)
{
    state->flags = FLAG_ABORT;
    if (state->buffer != NULL) {
        return 1;
    }
    return 0;
}

static void
stomp_network_acked(struct stomp_network_state *state)
{
    state->bufferlen -= state->sentlen;
    if (state->bufferlen == 0) {
        state->buffer = NULL;
        stomp_network_sent(state);
    } else {
        state->buffer += state->sentlen;
    }
    state->sentlen = 0;
}

void
stomp_network_app(void *s)
{
    struct stomp_network_state *state = (struct stomp_network_state*)s;
    
    if (uip_connected()) {
        state->flags = 0;
        stomp_network_connected(state);
        stomp_network_senddata(state);
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
    if (state->flags & FLAG_CLOSE) {
        uip_close();
        return;
    }
    if (state->flags & FLAG_ABORT) {
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