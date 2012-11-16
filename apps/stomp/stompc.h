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

/* TODO notifiing about sent data */
void
stompc_sent();

/* TODO notifiing about received data */
void
stompc_received(char *buf, uint16_t len);

#endif	/* STOMPC_H */