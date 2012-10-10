#ifndef STOMP_H_
#define STOMP_H_

#include "contiki.h"
#include "contiki-net.h"

void stomp_connect(char *host, uip_ipaddr_t *addr, uint16_t port, char* login, char* password);

void stomp_subscribe(char *id, char *dest, char *ack);

void stomp_unsubscribe(char *id);

void stomp_send(char *dest, char *type, char *len, char *receipt, char *tx, char *msg);

void stomp_begin(char *tx);

void stomp_commit(char *tx);

void stomp_abort(char *tx);

void stomp_disconnect(char *receipt);

#endif /* STOMP_H_ */
