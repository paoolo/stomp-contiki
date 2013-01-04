#include <string.h>

#include "stomp-strings.h"
#include "stomp-tools.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"
#include "stomp-network.h"

#define BUFFER_SIZE 256

#if UIP_CONF_IPV6 > 0
int addr[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
#else
int addr[] = {10, 1, 1, 100};
#endif

uip_ipaddr_t ipaddr;
int port = 61613;

PROCESS(stomp_network_test_process, "STOMP network test");
AUTOSTART_PROCESSES(&stomp_network_test_process, &stomp_network_process);

void
stomp_network_connected() {
    PRINTA("Connected.\n");
    process_post(&stomp_network_test_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_network_sent(char *buf, int len) {
    PRINTA("Sent:     {buf=\"%s\", len=\"%d\"}.\n", buf, len);
    process_post(&stomp_network_test_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_network_received(char *buf, int len) {
    PRINTA("Received: {buf=\"%s\", len=\"%d\"}.\n", buf, len);
}

static void
_rand_buffer(char *buffer, int size) {
    int i;

    memset(buffer, 0, size);
    for (i = 0; i < size; i++) {
        buffer[i] = rand() % ('z' - 'a') + 'a';
    }
}

int count = 0, i = 0, test = 0;
char *buffer;

PROCESS_THREAD(stomp_network_test_process, ev, data) {

    PROCESS_BEGIN();

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&ipaddr, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
#else
    uip_ipaddr(&ipaddr, addr[0], addr[1], addr[2], addr[3]);
#endif

    PRINTA("Start. Press any key...\n");
    getchar();

    PRINTA("Waiting for connection...\n");
    STOMP_NETWORK_CONNECT(&ipaddr, port);

    PRINTA("Test: Sending & Receiving.\n");
    for (test = 0; test < 10; test++) {
        for (count = 1; count < BUFFER_SIZE; count++) {
            buffer = NEW_ARRAY(char, count);
            _rand_buffer(buffer, count);
            stomp_network_send(buffer, count);
            DELETE(buffer);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
        }
    }

    PROCESS_END();
}