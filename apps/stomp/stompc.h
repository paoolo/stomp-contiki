#ifndef STOMPC_H
#define	STOMPC_H

#include "contiki.h"

#include "stomp-network.h"

extern const unsigned char stomp_version_default[4];
extern const unsigned char stomp_content_type_default[11];

struct stomp_state {
    struct pt pthread;
    struct psock socket;

    struct stomp_network_state network_state;

    unsigned char *host;
    unsigned char *login;
    unsigned char *passcode;
};

extern struct stomp_state state;

struct stomp_state*
stompc_connect(struct stomp_state *s, uip_ipaddr_t *addr, uint16_t port, unsigned char *host, unsigned char *login, unsigned char *passcode);

void
stompc_subscribe(struct stomp_state *s, unsigned char *id, unsigned char *dest, unsigned char *ack);

void
stompc_unsubscribe(struct stomp_state *s, unsigned char *id);

void
stompc_send(struct stomp_state *s, unsigned char *dest, unsigned char *type, unsigned char *len, unsigned char *receipt, char* tx, unsigned char *msg);

void
stompc_begin(struct stomp_state *s, unsigned char *tx);

void
stompc_commit(struct stomp_state *s, unsigned char *tx);

void
stompc_abort(struct stomp_state *s, unsigned char *tx);

void
stompc_disconnect(struct stomp_state *s, unsigned char *receipt);

#endif	/* STOMPC_H */

