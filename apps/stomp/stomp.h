#ifndef STOMP_H_
#define STOMP_H_

#include "contiki.h"

#if UIP_CONF_IPV6 > 0
void stomp_connect(char *host, uint16_t *addr, uint16_t port, char* login, char* password);
#else
void stomp_connect(char *host, uint8_t *addr, uint16_t port, char* login, char* password);
#endif

void stomp_send(char *dest, char *value, char *tx);

void stomp_subscribe(int id, char *dest, char *ack_mode);

void stomp_unsubscribe(int id);

void stomp_begin(char *tx);

void stomp_commit(char *tx);

void stomp_abort(char *tx);

void stomp_disconnect();

#endif /* STOMP_H_ */
