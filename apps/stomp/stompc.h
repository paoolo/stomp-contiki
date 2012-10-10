#ifndef STOMPC_H
#define	STOMPC_H

#include "stomp-network.h"

struct stomp_state {
    struct pt pthread;
    struct psock socket;
    struct stomp_network_state network_state;
    
    char *host;
    char *login;
    char *passcode;
};

extern struct stomp_state state;

struct stomp_state* stompc_connect(struct stomp_state *s, uip_ipaddr_t *addr, uint16_t port, char *host, char *login, char *passcode);

void stompc_subscribe(struct stomp_state *s, char *id, char *dest, char *ack);

void stompc_unsubscribe(struct stomp_state *s, char *id);

void stompc_send(struct stomp_state *s, char *dest, char *type, char *len, char *receipt, char* tx, char *msg);

void stompc_begin(struct stomp_state *s, char *tx);

void stompc_commit(struct stomp_state *s, char *tx);

void stompc_abort(struct stomp_state *s, char *tx);

void stompc_disconnect(struct stomp_state *s, char *receipt);

#endif	/* STOMPC_H */

