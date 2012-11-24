#include "ultra-simple-stomp.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"
#include "ultra-simple-stomp-network.h"

PROCESS(stomp_client_process, "STOMP client");
AUTOSTART_PROCESSES(&stomp_client_process, &ultra_simple_stomp_network_process);

PROCESS_THREAD(stomp_client_process, ev, data) {
    PROCESS_BEGIN();
    PRINTA("Start.\n");
    getchar();

    PROCESS_WAIT_EVENT();
    STOMP_CONNECT("apollo", "admin", "password");
    while (1) {
        STOMP_SUBSCRIBE("id-203284818775978", "/queue/203284818775978", "auto");
        STOMP_BEGIN("tx");
        STOMP_SEND("/queue/all", "text/plain", "5", NULL, NULL, "HELLO");
        STOMP_ABORT("tx");
        STOMP_BEGIN("tx");
        STOMP_SEND("/queue/all", "text/plain", "5", NULL, NULL, "HELLO");
        STOMP_COMMIT("tx");
        STOMP_UNSUBSCRIBE("id-203284818775978");
    }
    STOMP_DISCONNECT("0");

    PRINTA("Stop.\n");
    PROCESS_END();
}

void
stomp_sent() {
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_received(char *buf, int len) {
}

void
stomp_connected() {
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}