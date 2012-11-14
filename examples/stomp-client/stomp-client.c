#include "stomp.h"
#include "stompc.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static uip_ipaddr_t addr;
static int port = 61613;

static char *host = "apollo";

#if UIP_CONF_IPV6 > 0
static uint16_t host_ip[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
#else
static uint8_t host_ip[] = {10, 1, 1, 100};
#endif

PROCESS(stomp_client_process, "STOMP contiki client");
AUTOSTART_PROCESSES(&stomp_client_process, &stompc_process);

#define SEND_INTERVAL 15 * CLOCK_SECOND

PROCESS_THREAD(stomp_client_process, ev, data) {

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&addr, host_ip[0], host_ip[1], host_ip[2], host_ip[3], host_ip[4], host_ip[5], host_ip[6], host_ip[7]);
#else
    uip_ipaddr(&addr, host_ip[0], host_ip[1], host_ip[2], host_ip[3]);
#endif

    static struct etimer et;

    PROCESS_BEGIN();

    etimer_set(&et, CLOCK_CONF_SECOND * 3);
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    printf("Press any key...\n");
    getchar();

    stomp_connect(&addr, port, host, "admin", "password");

    stomp_send("/queue/a", "text/plain", NULL, NULL, NULL,
            "Testowa wiadomosc, wysylana na serwer");

    stomp_disconnect("0");

    printf("Press any key...\n");
    getchar();

    PROCESS_END();
}

void
stomp_sent() {
    printf("Yupie! It has been sent!\n");
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_received(char *buf, uint16_t len) {
    printf("Yeah! We have got it!\n");
}