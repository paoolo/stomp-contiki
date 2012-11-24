#include "contiki.h"
#include "contiki-net.h"

#ifndef ULTRA_SIMPLE_STOMP_H
#define	ULTRA_SIMPLE_STOMP_H

void
stomp_connect(char *host, char* login, char* pass);

#define STOMP_CONNECT(host, login, pass) \
        stomp_connect(host, login, pass); \
        PROCESS_WAIT_EVENT();

void
stomp_subscribe(char *id, char *destination, char *ack);

#define STOMP_SUBSCRIBE(id, destination, ack) \
        stomp_subscribe(id, destination, ack); \
        PROCESS_WAIT_EVENT();

void
stomp_unsubscribe(char *id);

#define STOMP_UNSUBSCRIBE(id) \
        stomp_unsubscribe(id); \
        PROCESS_WAIT_EVENT();

void
stomp_send(char *destination, char *type, char *length, char *receipt, char *tx, char *message);

#define STOMP_SEND(destination, type, length, receipt, tx, message) \
        stomp_send(destination, type, length, receipt, tx, message); \
        PROCESS_WAIT_EVENT();

void
stomp_ack(char *subscription, char *message_id, char *tx);

#define STOMP_ACK(subscription, message_id, tx) \
        stomp_ack(subscription, message_id, tx); \
        PROCESS_WAIT_EVENT();

void
stomp_nack(char *subscription, char *message_id, char *tx);

#define STOMP_NACK(subscription, message_id, tx) \
        stomp_nack(subscription, message_id, tx); \
        PROCESS_WAIT_EVENT();

void
stomp_begin(char *tx);

#define STOMP_BEGIN(tx) \
        stomp_begin(tx); \
        PROCESS_WAIT_EVENT();

void
stomp_commit(char *tx);

#define STOMP_COMMIT(tx) \
        stomp_commit(tx); \
        PROCESS_WAIT_EVENT();

void
stomp_abort(char *tx);

#define STOMP_ABORT(tx) \
        stomp_abort(tx); \
        PROCESS_WAIT_EVENT();

void
stomp_disconnect(char *receipt);

#define STOMP_DISCONNECT(receipt) \
        stomp_disconnect(receipt); \
        PROCESS_WAIT_EVENT();

void
stomp_sent();

void
stomp_received(char *buf, int len);

void
stomp_connected();

#endif	/* ULTRA_SIMPLE_STOMP_H */

