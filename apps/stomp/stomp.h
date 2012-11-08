#ifndef STOMP_H_
#define STOMP_H_

#include "contiki-net.h"

void
stomp_connect(uip_ipaddr_t *addr, uint16_t port, unsigned char *host, char* login, char* pass);

void
stomp_subscribe(unsigned char *id, unsigned char *dest, unsigned char *ack);

void
stomp_unsubscribe(unsigned char *id);

void
stomp_send(unsigned char *dest, unsigned char *type, unsigned char *len, unsigned char *receipt, unsigned char *tx, unsigned char *msg);

void
stomp_begin(unsigned char *tx);

void
stomp_commit(unsigned char *tx);

void
stomp_abort(unsigned char *tx);

void
stomp_disconnect(unsigned char *receipt);

#endif /* STOMP_H_ */
