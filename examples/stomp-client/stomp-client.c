#include "simple-stomp.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * main.c
 *
 * To jest demo programu klienckiego. Pokazuje, jak korzystac z
 * dostarczonych funkcji przez biblioteke obslugi polaczenia.
 * Protokolem uzywanym w polaczeniu, jest STOMP, wersja 1.1
 *
 *  Created on: 24-03-2012
 *      Author: paoolo
 *      Author: bibro
 */

static struct simple_stomp_state simple_state;

static unsigned char running = 1;
static unsigned int step = 0;

static uip_ipaddr_t addr;
static int port = 61613;

static char *host = "apollo";

#if UIP_CONF_IPV6 > 0
static uint16_t host_ip[] = {65152, 0, 0, 0, 0, 0, 0, 1};
#else
static uint8_t host_ip[] = {10, 1, 1, 100};
#endif

PROCESS(stompc_process, "STOMP contiki client");
AUTOSTART_PROCESSES(&stompc_process);

#define SEND_INTERVAL 15 * CLOCK_SECOND

PROCESS_THREAD(stompc_process, ev, data) {

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&addr, host_ip[0], host_ip[1], host_ip[2], host_ip[3], host_ip[4], host_ip[5], host_ip[6], host_ip[7]);
#else
    uip_ipaddr(&addr, host_ip[0], host_ip[1], host_ip[2], host_ip[3]);
#endif

    static struct etimer et;

    PROCESS_BEGIN();

    uip_init();

    // wait 3 second, in order to have the IP addresses well configured
    etimer_set(&et, CLOCK_CONF_SECOND * 3);

    // wait until the timer has expired
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    printf("Press any key...\n");

    getchar();

    simple_connect(&simple_state, &addr, port, host, "admin", "password");

#ifdef WITH_UDP
    etimer_set(&et, SEND_INTERVAL);
    while (running) {
        PROCESS_YIELD();
        if (etimer_expired(&et)) {
            simple_app(&simple_state);
            etimer_restart(&et);
        } else if (ev == tcpip_event) {
            simple_app(&simple_state);
        }

        switch (step) {
            case 0:
                step = step + 1;
                simple_connected(&simple_state);
                break;

            case 1:
                step = step + 1;
                simple_stomp_send(&simple_state, "/queue/a", "text/plain", NULL, NULL, NULL, "Testowa wiadomosc, wysylana na serwer");
                break;

            case 2:
                step = step + 1;
                simple_stomp_disconnect(&simple_state, "0");
                break;

            default:
                break;
        }
    }
#else
    while (running) {
        PROCESS_WAIT_EVENT();
        simple_app(&simple_state);

        switch (step) {
            case 0:
                step = step + 1;
                simple_stomp_send(&simple_state, "/queue/a", "text/plain", NULL, NULL, NULL, "Testowa wiadomosc, wysylana na serwer");
                break;

            case 1:
                step = step + 1;
                simple_stomp_disconnect(&simple_state, "0");
                break;

            default:
                break;
        }

    }
#endif
    PROCESS_END();
}
