/* 
 * File:   simple-stomp.h
 * Author: paoolo
 *
 * Created on 10 październik 2012, 21:30
 */

#ifndef SIMPLE_STOMP_H
#define	SIMPLE_STOMP_H

#include "contiki.h"
#include "contiki-net.h"

extern const int _C_LONG_SIZE;

/* Magic number :) */
#define C_LONG_SIZE _C_LONG_SIZE
#define JAVA_LONG_SIZE 8

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

    uip_ipaddr_t *ipaddr;
    uint16_t port;
    
#ifdef WITH_UDP
    struct uip_udp_conn *conn;
#else
    struct uip_conn *conn;
#endif

    struct stomp_frame *frame;

    unsigned char *host;
    unsigned char *login;
    unsigned char *pass;
};

void
simple_app(void *s);

struct simple_stomp_state *
simple_connect(struct simple_stomp_state *s, uip_ipaddr_t *ipaddr, uint16_t port, unsigned char *host, unsigned char *login, unsigned char *pass);

void
simple_connected(struct simple_stomp_state *s);

void
simple_stomp_subscribe(struct simple_stomp_state *s, unsigned char *id, unsigned char *destination, unsigned char *ack);

void
simple_stomp_unsubscribe(struct simple_stomp_state *s, unsigned char *id);

void
simple_stomp_send(struct simple_stomp_state *s, unsigned char *destination, unsigned char *type, unsigned char *length, unsigned char *receipt, unsigned char *tx, unsigned char *message);

void
simple_stomp_begin(struct simple_stomp_state *s, unsigned char *tx);

void
simple_stomp_commit(struct simple_stomp_state *s, unsigned char *tx);

void
simple_stomp_abort(struct simple_stomp_state *s, unsigned char *tx);

void
simple_stomp_disconnect(struct simple_stomp_state *s, unsigned char *receipt);

void
simple_disconnected(struct simple_stomp_state *);

#endif	/* SIMPLE_STOMP_H */

