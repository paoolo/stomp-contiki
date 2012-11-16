#ifndef STOMP_H_
#define STOMP_H_

#include "stomp-frame.h"

#include <stdint.h>

// #define WITH_UDP

extern const char stomp_version_default[4];
extern const char stomp_content_type_default[11];

void
stomp_connect(char *host, char* login, char* pass);

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

void
stomp_sent();

void
stomp_received(struct stomp_frame *frame);

#endif /* STOMP_H_ */
