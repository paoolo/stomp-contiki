#include "stompc.h"

#include "contiki-net.h"

#include <stdlib.h>
#include <stdio.h>

void
stomp_connect(uip_ipaddr_t *addr, uint16_t port, char* host, char* login, char* pass)
{
    printf("CONNECT: host=%s, login=%s, pass=%s\n", host, login, pass);
    stompc_connect(&state, addr, port, host, login, pass);
}

void
stomp_subscribe(unsigned char *id, unsigned char *dest, unsigned char *ack)
{
    printf("SUBSCRIBE: id=%s, destination=%s, ack=%s\n", id, dest, ack);
    stompc_subscribe(&state, id, dest, ack);
}

void
stomp_unsubscribe(unsigned char *id)
{
    printf("UNSUBSCRIBE: id=%s\n", id);
    stompc_unsubscribe(&state, id);
}

void
stomp_send(unsigned char *dest, unsigned char *type, unsigned char *len, unsigned char *receipt, unsigned char *tx, unsigned char *msg)
{
    printf("SEND: dest=%s, type=%s, len=%s, receipt=%s, tx=%s, msg=%s\n", dest, type, len, receipt, tx, msg);
    stompc_send(&state, dest, type, len, receipt, tx, msg);
}

void
stomp_begin(unsigned char *tx)
{
    printf("BEGIN: tx=%s\n", tx);
    stompc_begin(&state, tx);
}

void
stomp_commit(unsigned char *tx)
{
    printf("COMMIT: tx=%s\n", tx);
    stompc_commit(&state, tx);
}

void
stomp_abort(unsigned char *tx)
{
    printf("ABORT: tx=%s\n", tx);
    stompc_abort(&state, tx);
}

void
stomp_disconnect(unsigned char *receipt)
{
    printf("DISCONNECT: receipt=%s\n", receipt);
    stompc_disconnect(&state, receipt);
}