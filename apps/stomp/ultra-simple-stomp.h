#include "contiki.h"
#include "contiki-net.h"

#ifndef ULTRA_SIMPLE_STOMP_H
#define	ULTRA_SIMPLE_STOMP_H

#define ULTRA_SIMPLE_STOMP_FLAG_DISCONNECT 1
#define ULTRA_SIMPLE_STOMP_FLAG_ABORT 2

#define ULTRA_SIMPLE_STOMP_BUF_SIZE 256

struct ultra_simple_stomp {
#ifdef WITH_UDP
    struct uip_udp_conn *conn;
#else
    struct uip_conn *conn;
#endif

    uip_ipaddr_t addr;
    int port;
    char flags;

    char *buf;
    int off;
    int len;
    int sentlen;
};

extern struct ultra_simple_stomp state;

void
stomp_app();

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

/* Ultra-simple-stomp network section */

void
stomp_net_connect();

#ifndef WITH_UDP
void
stomp_net_connected();

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
stomp_net_send();

void
stomp_net_sent();

void
stomp_net_received(char *buf, int len);

#endif	/* ULTRA_SIMPLE_STOMP_H */

