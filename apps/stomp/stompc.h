#ifndef STOMPC_H
#define	STOMPC_H

#include "contiki.h"

#include "stomp-network.h"

#define WITH_UDP

extern const char stomp_version_default[4];
extern const char stomp_content_type_default[11];

struct stomp_state {
    struct stomp_network_state network_state;

    struct stomp_frame *frame;

    char *host;
    char *login;
    char *pass;
};

extern struct stomp_state state;

PROCESS_NAME(stompc_process);

void
stompc_app(struct stomp_state *s);

struct stomp_state*
stompc_connect(struct stomp_state *s, uip_ipaddr_t *addr, uint16_t port, char *host, char *login, char *pass);

void
stompc_subscribe(struct stomp_state *s, char *id, char *destination, char *ack);

void
stompc_unsubscribe(struct stomp_state *s, char *id);

void
stompc_send(struct stomp_state *s, char *destination, char *type, char *lenght, char *receipt, char* tx, char *message);

void
stompc_begin(struct stomp_state *s, char *tx);

void
stompc_commit(struct stomp_state *s, char *tx);

void
stompc_abort(struct stomp_state *s, char *tx);

void
stompc_disconnect(struct stomp_state *s, char *receipt);

/* Callbacks */

void
stompc_sent();

void
stompc_received(char *buf, uint16_t len);

#endif	/* STOMPC_H */