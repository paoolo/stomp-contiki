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

#define SIMPLE_STOMP_FLAG_CLOSE 1
#define SIMPLE_STOMP_FLAG_ABORT 2

struct simple_stomp_state
{
    struct pt pt;
    struct psock s;
    
    unsigned char flags;
    
    char outputbuf[200];
    char inputbuf[400];
    
    struct uip_conn *conn;
    
    struct stomp_frame *frame;
    
    char *host;
    char *login;
    char *pass;
};

extern struct simple_stomp_state simple_state;

void simple_app(void *s);

struct simple_stomp_state *
simple_connect(struct simple_stomp_state *s, uip_ipaddr_t *ipaddr, uint16_t port, char *host, char *login, char *pass);

void
simple_connected(struct simple_stomp_state *s);

void
simple_stomp_subscribe(struct simple_stomp_state *s, char *id, char *destination, char *ack);

void
simple_stomp_unsubscribe(struct simple_stomp_state *s, char *id);

void
simple_stomp_send(struct simple_stomp_state *s, char *destination, char *type, char *length, char *receipt, char *tx, char *message);

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

