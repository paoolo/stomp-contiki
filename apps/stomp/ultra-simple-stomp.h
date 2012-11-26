#include "contiki.h"
#include "contiki-net.h"

#ifndef ULTRA_SIMPLE_STOMP_H
#define	ULTRA_SIMPLE_STOMP_H

void
stomp_connect(struct process *proc, char *host, char* login, char* pass);

#define STOMP_CONNECT(host, login, pass) \
        stomp_connect(PROCESS_CURRENT(), host, login, pass); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_subscribe(struct process *proc, char *id, char *destination, char *ack);

#define STOMP_SUBSCRIBE(id, destination, ack) \
        stomp_subscribe(PROCESS_CURRENT(), id, destination, ack); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_unsubscribe(struct process *proc, char *id);

#define STOMP_UNSUBSCRIBE(id) \
        stomp_unsubscribe(PROCESS_CURRENT(), id); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_send(struct process *proc, char *destination, char *type, char *length, char *receipt, char *tx, char *message);

#define STOMP_SEND(destination, type, length, receipt, tx, message) \
        stomp_send(PROCESS_CURRENT(), destination, type, length, receipt, tx, message); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_ack(struct process *proc, char *subscription, char *message_id, char *tx);

#define STOMP_ACK(subscription, message_id, tx) \
        stomp_ack(PROCESS_CURRENT(), subscription, message_id, tx); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_nack(struct process *proc, char *subscription, char *message_id, char *tx);

#define STOMP_NACK(subscription, message_id, tx) \
        stomp_nack(PROCESS_CURRENT(), subscription, message_id, tx); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_begin(struct process *proc, char *tx);

#define STOMP_BEGIN(tx) \
        stomp_begin(PROCESS_CURRENT(), tx); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_commit(struct process *proc, char *tx);

#define STOMP_COMMIT(tx) \
        stomp_commit(PROCESS_CURRENT(), tx); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_abort(struct process *proc, char *tx);

#define STOMP_ABORT(tx) \
        stomp_abort(PROCESS_CURRENT(), tx); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_disconnect(struct process *proc, char *receipt);

#define STOMP_DISCONNECT(receipt) \
        stomp_disconnect(PROCESS_CURRENT(), receipt); \
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

void
stomp_sent(char *buf, int len);

void
stomp_received(char *buf, int len);

void
stomp_connected(char *version, char *server, char *host_id, char *session, char *heart_beat, char *user_id);

void
stomp_message(char *destination, char *message_id, char *subscription, char *content_type, char *content_length, char *message);

void
stomp_error(char *receipt_id, char *content_type, char *content_length, char *message);

void
stomp_receipt(char *receipt_id);

#endif	/* ULTRA_SIMPLE_STOMP_H */

