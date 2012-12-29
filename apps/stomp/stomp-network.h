#ifndef STOMP_NETWORK_H
#define STOMP_NETWORK_H

#include "contiki.h"
#include "contiki-net.h"

#define STOMP_FLAG_DISCONNECT 1
#define STOMP_FLAG_ABORT 2

#define WITH_UDP

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

PROCESS_NAME(stomp_network_send_process);
PROCESS_NAME(stomp_network_process);

void
stomp_network_connect(uip_ipaddr_t *ipaddr, int port);

void
stomp_network_connected();

#ifndef WITH_UDP
void
stomp_network_timedout();

void
stomp_network_abort();

void
stomp_network_aborted();

void
stomp_network_close();

void
stomp_network_closed();
#endif

void
stomp_network_send(char *buf, int len);

void
stomp_network_sent(char *buf, int len);

void
stomp_network_received(char *buf, int len);

#endif /* STOMP_NETWORK_H */