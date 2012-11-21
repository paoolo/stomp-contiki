#ifndef STOMPC_H
#define	STOMPC_H

#include "contiki.h"

#include "stomp.h"
#include "stomp-frame.h"

extern const char stomp_version_default[4];
extern const char stomp_content_type_default[11];

struct stompc_state {
    struct stomp_frame *frame;
};

extern struct stompc_state c_state;

PROCESS_NAME(stompc_process);

void
stompc_frame();

/* TODO notifing about connected */
void
stompc_connected();

/* TODO notifing about sent data */
void
stompc_sent();

/* TODO notifing about received data */
void
stompc_received(char *buf, uint16_t len);

/* TODO notifing about closed */
void
stompc_closed();

#endif	/* STOMPC_H */