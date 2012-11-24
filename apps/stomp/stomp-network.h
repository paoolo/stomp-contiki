#include "stomp-global.h"

#ifndef STOMP_NETWORK_H
#define STOMP_NETWORK_H

#include "contiki.h"
#include "contiki-net.h"

#define STOMP_FLAG_DISCONNECT 1
#define STOMP_FLAG_ABORT 2

struct stomp_network_state {
#ifdef WITH_UDP
    struct uip_udp_conn *conn;
#else
    struct uip_conn *conn;
#endif

    uip_ipaddr_t *addr;
    int port;
    char flags;

    char *buf;
    int off;
    int len;
    int sentlen;
};

extern struct stomp_network_state network_state;

extern struct stomp_queue network_send_queue;

extern uip_ipaddr_t stomp_network_addr;
extern int stomp_network_port;

#if UIP_CONF_IPV6 > 0
extern int stomp_network_addr_num[];
#else
extern uint8_t stomp_network_addr_num[];
#endif

PROCESS_NAME(stomp_network_process);
PROCESS_NAME(stomp_network_send_process);

unsigned char
stomp_network_send(char *buf, int len);

#ifndef WITH_UDP
unsigned char
stomp_network_close();
#endif

#ifndef WITH_UDP
unsigned char
stomp_network_abort();
#endif

/* TODO protocol negotation and registering to server */
#ifndef WITH_UDP
void
stomp_network_connected();
#endif

/* TODO notyfing about sent message to server */
#ifndef WITH_UDP
void
stomp_network_sent();
#endif

/* TODO parsing frame and to do something with this */
void
stomp_network_received(char *buf, int len);

/* TODO clean up session */
#ifndef WITH_UDP
void
stomp_network_closed();
#endif

/* TODO notifing about aborted connection */
#ifndef WITH_UDP
void
stomp_network_aborted();
#endif

/* TODO notifing about timedout connection */
#ifndef WITH_UDP
void
stomp_network_timedout();
#endif

#endif /* STOMP_NETWORK_H */