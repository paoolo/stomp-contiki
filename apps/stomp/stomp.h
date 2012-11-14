#ifndef STOMP_H_
#define STOMP_H_

#include "contiki-net.h"

#define WITH_UDP

void
stomp_app();

void
stomp_connect(uip_ipaddr_t *addr, uint16_t port, char *host, char* login, char* pass);

void
stomp_subscribe(char *id, char *destination, char *ack);

void
stomp_unsubscribe(char *id);

void
stomp_send(char *destination, char *type, char *length, char *receipt, char *tx, char *message);

void
stomp_begin(char *tx);

void
stomp_commit(char *tx);

void
stomp_abort(char *tx);

void
stomp_disconnect(char *receipt);

/* Callbacks */

void
stomp_sent();

void
stomp_received(char *buf, uint16_t len);

#endif /* STOMP_H_ */
