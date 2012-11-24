
#ifndef STOMPC_H
#define	STOMPC_H

extern const char stomp_version_default[4];
extern const char stomp_content_type_default[11];

struct stompc_state {
    struct stomp_frame *frame;
};

extern struct stompc_state c_state;

void
stompc_frame();

#ifndef WITH_UDP
/* TODO notifing about connected */
void
stompc_connected();
#endif

#ifndef WITH_UDP
/* TODO notifing about sent data */
void
stompc_sent();
#endif

/* TODO notifing about received data */
void
stompc_received(char *buf, int len);

#ifndef WITH_UDP
/* TODO notifing about closed */
void
stompc_closed();
#endif

#endif	/* STOMPC_H */