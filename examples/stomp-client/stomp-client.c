#include "stomp.h"
#include "stompc.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "stomp-network.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PROCESS(stomp_client_process, "STOMP contiki client");
AUTOSTART_PROCESSES(&stomp_client_process, &stompc_process, &stomp_network_process);

PROCESS_THREAD(stomp_client_process, ev, data) {
    PROCESS_BEGIN();

    printf("Press any key...\n");
    getchar();

    stomp_connect("apollo", "admin", "password");
    PROCESS_WAIT_EVENT();
    
    stomp_send("/queue/a", "text/plain", NULL, NULL, NULL, "Testowa wiadomosc, wysylana na serwer");
    PROCESS_WAIT_EVENT();
    
    stomp_disconnect("0");
    PROCESS_WAIT_EVENT();

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
stomp_received(struct stomp_frame *frame) {
    printf("Yeah! We have got it!\n");
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}