#ifndef NETWORK_H_
#define NETWORK_H_

#include "contiki.h"
#include "contiki-net.h"

#include "stomp.h"

#define STOMP_FLAG_DISCONNECT 1
#define STOMP_FLAG_ABORT 2

struct stomp_network_state {
#ifdef WITH_UDP
    struct uip_udp_conn *conn;
#else
    struct uip_conn *conn;
#endif

    uip_ipaddr_t *addr;
    uint16_t port;
    char flags;

    char *buf;
    uint16_t off;
    uint16_t len;
    uint16_t sentlen;
};

extern struct stomp_network_state network_state;

extern struct stomp_queue network_send_queue;

extern uip_ipaddr_t stomp_network_addr;
extern int stomp_network_port;

#if UIP_CONF_IPV6 > 0
extern uint16_t stomp_network_addr_num[];
#else
extern uint8_t stomp_network_addr_num[];
#endif

PROCESS_NAME(stomp_network_process);
PROCESS_NAME(stomp_network_send_process);

unsigned char
stomp_network_send(char *buf, uint16_t len);

unsigned char
stomp_network_close();

unsigned char
stomp_network_abort();

/* TODO protocol negotation and registering to server */
void
stomp_network_connected();

/* TODO notyfing about sent message to server */
void
stomp_network_sent();

/* TODO parsing frame and to do something with this */
void
stomp_network_received(char *buf, uint16_t len);

/* TODO clean up session */
void
stomp_network_closed();

/* TODO notifing about aborted connection */
void
stomp_network_aborted();

/* TODO notifing about timedout connection */
void
stomp_network_timedout();

#endif /* NETWORK_H_ */