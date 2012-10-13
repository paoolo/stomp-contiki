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

PROCESS(stompc_process, "STOMP contiki client");
AUTOSTART_PROCESSES(&stompc_process);

/* Glowny program */
PROCESS_THREAD(stompc_process, ev, data) {
    
    unsigned char running = 1;
    unsigned int step = 0;
    int port = 61613;

#if UIP_CONF_IPV6 > 0
    char *host = "aaaa:0000:0000:0000:0000:0000:0000:0001";
    uint16_t host_ip[] = { 43690, 0, 0, 0, 0, 0, 0, 1 };
#else
    char *host = "10.1.1.100";
    uint8_t host_ip[] = { 10, 1, 1, 100 };
#endif
    
    uip_ipaddr_t addr;
    uip_ipaddr(&addr, host_ip[0], host_ip[1], host_ip[2], host_ip[3]);
    
    PROCESS_BEGIN();

    getchar();

    uip_init();
    
    simple_connect(&simple_state, &addr, port, host, "admin", "password");
    
    while (running) {
        PROCESS_WAIT_EVENT();
        
        if (ev == tcpip_event) {
            simple_app(&simple_state);
        }
        
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
                running = 0;
                break;

        }
    }
    PROCESS_END();
}
