#include "stomp-global.h"

#include "stomp-frame.h"

#ifndef STOMP_H
#define STOMP_H

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

#ifndef WITH_UDP
void
stomp_connected();
#endif

#ifndef WITH_UDP
void
stomp_sent();
#endif

void
stomp_received(struct stomp_frame *frame);

#ifndef WITH_UDP
void
stomp_closed();
#endif

#endif /* STOMP_H */
