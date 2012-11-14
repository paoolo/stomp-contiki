#include "stomp.h"

#ifndef NETWORK_H_
#define NETWORK_H_

#include "contiki.h"
#include "contiki-net.h"

#define STOMP_FLAG_CLOSE 1
#define STOMP_FLAG_ABORT 2

#define STOMP_OUTPUTBUF_SIZE 512
#define STOMP_INPUTBUF_SIZE 512

struct stomp_network_state {
    struct pt pt;
    struct psock s;

    uint16_t outputbuf_len;
    uint16_t outputbuf_sentlen;
    uint8_t outputbuf[STOMP_OUTPUTBUF_SIZE];

    uint16_t inputbuf_len;
    uint16_t inputbuf_sentlen;
    uint8_t inputbuf[STOMP_INPUTBUF_SIZE];

#ifdef WITH_UDP
    struct uip_udp_conn *conn;
#else
    struct uip_conn *conn;
#endif

    uip_ipaddr_t *addr;
    uint16_t port;
    char flags;
    
    char *buf;
    uint16_t buflen;
    uint16_t sentlen;
};

void
stomp_network_app(void *s);

struct stomp_state*
stomp_network_connect(struct stomp_network_state *s, uip_ipaddr_t *addr, uint16_t port);

/* Callbacks, must be implemented. */

/* TODO protocol negotation and registering to server */
void
stomp_network_connected(struct stomp_network_state *s);

/* TODO notyfing about sent message to server */
void
stomp_network_sent(struct stomp_network_state *s);

/* TODO parsing frame and to do something with this */
void
stomp_network_received(struct stomp_network_state *s, char *buf, uint16_t len);

/* TODO clean up session */
void
stomp_network_closed(struct stomp_network_state *s);

/* TODO notifing about aborted connection */
void
stomp_network_aborted(struct stomp_network_state *s);

/* TODO notifing about timedout connection */
void
stomp_network_timedout(struct stomp_network_state *s);


#endif /* NETWORK_H_ */