
#include <malloc.h>

#include "contiki.h"
#include "contiki-net.h"

#include "stomp-network.h"
#include "stompc.h"
#include "stomp.h"

enum {
    COMMAND_NONE,
    COMMAND_CONNECT,
    COMMAND_SUBSCRIBE,
    COMMAND_UNSUBSCRIBE,
    COMMAND_SEND,
    COMMAND_BEGIN,
    COMMAND_COMMIT,
    COMMAND_ABORT,
    COMMAND_DISCONNECT
};

static
PT_THREAD(data_or_command(struct stomp_state *s))
{
    PSOCK_BEGIN(&state->socket);
    
    PSOCK_WAIT_UNTIL(&state->socket, PSOCK_NEWDATA(&state->socket) || 
            (state->command != COMMAND_NONE));
    
    PSOCK_END(&state->socket);
}

#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, (unsigned int)strlen(str))

static
PT_THREAD(setup_connection(struct stomp_state *state))
{
    char *ptr;
    
    PSOCK_BEGIN(&state->socket);
    
    SEND_STRING(&state->socket, state->outputbuf);
    
    PSOCK_END(&state->socket);
}

static
PT_THREAD(handle_connection(struct stomp_state *state))
{
    PT_BEGIN(&state->pthread);
    
    PSOCK_INIT(&state->socket, (uint8_t*)(state->inputbuf), sizeof(state->inputbuf)-1);
    
    PT_WAIT_THREAD(&state->pthread, setup_connection(state));
    
    while(1) {
        
        PT_WAIT_UNTIL(&state->pthread, data_or_command(state));
        
        if (PSOCK_NEWDATA(&state->socket)) {
            PT_WAIT_THREAD(&state->pthread, handle_input(state));
        }
        
        // TODO what do?
    }
    
    PT_END(&state->pthread);
}

static
PT_THREAD(stomp_send_connect(struct stomp_state *state))
{
    PSOCK_BEGIN(&state->socket);
    
    SEND_STRING(&state->socket, _FRAME_CMD_CONNECT);
    
    PSOCK_END(&state->socket);
}

void
stomp_init(void)
{
    
}

void
stomp_appcall(void *s)
{
    if (uip_closed() || uip_aborted() || uip_timedout()) {
        stomp_disconnected(s);
        
    } else if (uip_connected()) {
        struct stomp_state *state = (struct stomp_state*)s;
        stomp_connected(state);
        
        PT_INIT(&(state->pthread));
        memset(state->command, 0, sizeof(state->command));
        state->command = COMMAND_NONE;
        
        handle_connection(state);
        
    } else if (state != NULL) {
        handle_connection(s);
    }
}

struct stomp_state *
stomp_connect(struct stomp_state *state, char *hostname, uip_ipaddr_t *host, uint16_t port)
{
    if (stomp_network_connect(&state->network_state, host, port) == NULL) {
        return NULL;
    }

    state->network_state.hostname = hostname;
    return state;
}

void
stomp_subscribe(struct stomp_state *state, unsigned char id, char *destination)
{
    state->command = COMMAND_SUBSCRIBE;
    state->id = id;
    state->destination = destination;
}

void
stomp_unsubscribe(struct stomp_state *state, unsigned char id)
{
    state->command = COMMAND_UNSUBSCRIBE;
    state->id = id;
}

void
stomp_send(struct stomp_state *state, char *destination, char *message, char *tx)
{
    state->command = COMMAND_SEND;
    state->destination = destination;
    state->message = message;
    state->tx = tx;
}

void
stomp_begin(struct stomp_state *state, char *tx)
{
    state->command = COMMAND_BEGIN;
    state->tx = tx;
}

void
stomp_commit(struct stomp_state *state, char *tx)
{
    state->command = COMMAND_COMMIT;
    state->tx = tx;
}

void
stomp_abort(struct stomp_state *state, char *tx)
{
    state->command = COMMAND_ABORT;
    state->tx = tx;
}

void
stomp_disconnect(struct stomp_state *state)
{
    state->command = COMMAND_DISCONNECT;
}