#ifndef NETWORK_H_
#define NETWORK_H_

#include "contiki.h"
#include "contiki-net.h"

#define STOMP_FLAG_CLOSE 1
#define STOMP_FLAG_ABORT 2

struct stomp_network_state {
    struct uip_conn *conn;

    uip_ipaddr_t *address;
    uint16_t port;

    unsigned char flags;
    unsigned char *buffer;
    uint16_t bufferlen;
    uint16_t sentlen;
};

void
stomp_network_app(void *s);

struct stomp_state*
stomp_network_connect(struct stomp_state *s, uip_ipaddr_t *addr, uint16_t port);

unsigned char
stomp_network_send(struct stomp_state *s, unsigned char *buf, uint16_t len);

unsigned char
stomp_network_close(struct stomp_state *s);

unsigned char
stomp_network_abort(struct stomp_state *s);

/*
 * Callbacks, must be implemented.
 */

/* TODO protocol negotation and registering to server */
void
stomp_network_connected(struct stomp_state *s);

/* TODO notyfing about sent message to server */
void
stomp_network_sent(struct stomp_state *s);

/* TODO parsing frame and to do something with this */
void
stomp_network_received(struct stomp_state *s, char *buf, uint16_t len);

/* TODO clean up session */
void
stomp_network_closed(struct stomp_state *s);

/* TODO notifing about aborted connection */
void
stomp_network_aborted(struct stomp_state *s);

/* TODO notifing about timedout connection */
void
stomp_network_timedout(struct stomp_state *s);


#endif /* NETWORK_H_ */