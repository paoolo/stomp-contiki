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
stomp_subscribe(char *id, char *dest, char *ack)
{
    printf("SUBSCRIBE: id=%s, destination=%s, ack=%s\n", id, dest, ack);
    stompc_subscribe(&state, id, dest, ack);
}

void
stomp_unsubscribe(char *id)
{
    printf("UNSUBSCRIBE: id=%s\n", id);
    stompc_unsubscribe(&state, id);
}

void
stomp_send(char *dest, char *type, char *len, char *receipt, char *tx, char *msg)
{
    printf("SEND: dest=%s, type=%s, len=%s, receipt=%s, tx=%s, msg=%s\n", dest, type, len, receipt, tx, msg);
    stompc_send(&state, dest, type, len, receipt, tx, msg);
}

void
stomp_begin(char *tx)
{
    printf("BEGIN: tx=%s\n", tx);
    stompc_begin(&state, tx);
}

void
stomp_commit(char *tx)
{
    printf("COMMIT: tx=%s\n", tx);
    stompc_commit(&state, tx);
}

void
stomp_abort(char *tx)
{
    printf("ABORT: tx=%s\n", tx);
    stompc_abort(&state, tx);
}

void
stomp_disconnect(char *receipt)
{
    printf("DISCONNECT: receipt=%s\n", receipt);
    stompc_disconnect(&state, receipt);
}