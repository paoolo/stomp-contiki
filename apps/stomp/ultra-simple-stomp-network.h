#include "contiki.h"
#include "contiki-net.h"

#ifndef ULTRA_SIMPLE_STOMP_NETWORK_H
#define	ULTRA_SIMPLE_STOMP_NETWORK_H

#define ULTRA_SIMPLE_STOMP_FLAG_DISCONNECT 1
#define ULTRA_SIMPLE_STOMP_FLAG_ABORT 2

#define ULTRA_SIMPLE_STOMP_BUF_SIZE 256

#define WITH_UDP

struct ultra_simple_stomp {
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

extern struct ultra_simple_stomp state;

PROCESS_NAME(ultra_simple_stomp_network_process);

void
stomp_net_connect(uip_ipaddr_t *ipaddr, int port);

void
stomp_net_connected();

#ifndef WITH_UDP
void
stomp_net_timedout();

void
stomp_net_abort();

void
stomp_net_aborted();

void
stomp_net_close();

void
stomp_net_closed();
#endif

void
stomp_net_send(struct process *proc, char *buf, int len);

void
stomp_net_sent();

void
stomp_net_received(char *buf, int len);

#endif	/* ULTRA_SIMPLE_STOMP_NETWORK_H */

