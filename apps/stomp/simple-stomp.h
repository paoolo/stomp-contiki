#ifndef SIMPLE_STOMP_H
#define	SIMPLE_STOMP_H

#include "contiki.h"
#include "contiki-net.h"

#define SIMPLE_STOMP_FLAG_CLOSE 1
#define SIMPLE_STOMP_FLAG_ABORT 2

#define SIMPLE_STOMP_OUTPUTBUF_SIZE 512
#define SIMPLE_STOMP_INPUTBUF_SIZE 512

#define WITH_UDP

struct simple_stomp_state {
    struct pt pt;
    struct psock s;

    unsigned int outputbuf_len;
    uint8_t outputbuf[SIMPLE_STOMP_OUTPUTBUF_SIZE];
    uint8_t inputbuf[SIMPLE_STOMP_INPUTBUF_SIZE];

    uip_ipaddr_t *addr;
    uint16_t port;

#ifdef WITH_UDP
    struct uip_udp_conn *conn;
#else
    struct uip_conn *conn;
#endif

    struct stomp_frame *frame;

    char *host;
    char *login;
    char *pass;
};

void
simple_app(void *s);

struct simple_stomp_state *
simple_connect(struct simple_stomp_state *s, uip_ipaddr_t *addr, uint16_t port,
        char *host, char *login, char *pass);

void
simple_connected(struct simple_stomp_state *s);

void
simple_stomp_subscribe(struct simple_stomp_state *s, char *id, char *destination,
        char *ack);

void
simple_stomp_unsubscribe(struct simple_stomp_state *s, char *id);

void
simple_stomp_send(struct simple_stomp_state *s, char *destination, char *type,
        char *length, char *receipt, char *tx, char *message);

void
simple_stomp_begin(struct simple_stomp_state *s, char *tx);

void
simple_stomp_commit(struct simple_stomp_state *s, char *tx);

void
simple_stomp_abort(struct simple_stomp_state *s, char *tx);

void
simple_stomp_disconnect(struct simple_stomp_state *s, char *receipt);

void
simple_disconnected(struct simple_stomp_state *);

#endif	/* SIMPLE_STOMP_H */