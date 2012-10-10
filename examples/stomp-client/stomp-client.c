#include "stomp-memguard.h"
#include "stomp-network.h"
#include "stomp-frame.h"
#include "stomp.h"

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

PROCESS(program, "STOMP contiki client");
AUTOSTART_PROCESSES(&program);

/* Glowny program */
PROCESS_THREAD(program, ev, data) {
    PROCESS_BEGIN();

    getchar();

    int port = 61613;

#if UIP_CONF_IPV6 > 0
    char *host = "aaaa:0000:0000:0000:0000:0000:0000:0001";
    uint16_t host_ip[] = { 43690, 0, 0, 0, 0, 0, 0, 1 };
#else
    char *host = "10.1.1.100";
    uint8_t host_ip[] = { 10, 1, 1, 100 };
#endif

    uip_init();
    stomp_memguard_init();

    stomp_connect(host, host_ip, port, "admin", "password");
    stomp_subscribe(0, "/queue/a", "client");
    stomp_send("/queue/a", "Testowa wiadomosc, wysylana na serwer", NULL);
    stomp_unsubscribe(0);
    stomp_disconnect(0);

    PROCESS_END();
}
