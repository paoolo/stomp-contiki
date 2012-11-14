#include "stompc.h"

#include "contiki.h"
#include "contiki-net.h"

#include "stomp-tools.h"
#include "stomp-frame.h"
#include "stomp-network.h"
#include "stomp-strings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char stomp_version_default[4] = {0x31, 0x2e, 0x31,};

const char stomp_content_type_default[11] = {0x74, 0x65, 0x78, 0x74, 0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e,};

struct stomp_state state;

#define STOMPC_INTERVAL 15 * CLOCK_SECOND

PROCESS(stompc_process, "STOMPc process");
AUTOSTART_PROCESSES(&stompc_process);

PROCESS_THREAD(stompc_process, ev, data) {
    static struct etimer et;

    PROCESS_BEGIN();

    printf("stompc_process: start.\n");
#ifdef WITH_UDP
    etimer_set(&et, STOMPC_INTERVAL);
    while (1) {
        PROCESS_WAIT_EVENT();
        if (etimer_expired(&et)) {
            printf("stompc_process: interval expired.\n");
            stompc_app(&state);
            etimer_restart(&et);

        } else {
            printf("stompc_process: any event.\n");
            stompc_app(&state);
        }
    }
#else
    while (1) {
        PROCESS_WAIT_EVENT();
        printf("stompc_process: event.\n");
        stompc_app(&state);
    }
#endif
    printf("stompc_process: stop.\n");

    PROCESS_END();
}

void
stompc_app(struct stomp_state * s) {
    printf("stompc_app: start.\n");

    stomp_network_app(s);

    printf("stompc_app: stop.\n");
}

/* Forming proper stomp frame */

struct stomp_state *
stompc_connect(struct stomp_state *state, uip_ipaddr_t *addr, uint16_t port, char *host, char *login, char *pass) {
    printf("stompc_connect: start.\n");

    if (stomp_network_connect(state, addr, port) == NULL) {
        printf("Not connected.\n");
        return NULL;
    }

    state->host = host;
    state->login = login;
    state->pass = pass;
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stompc_connect: stop.\n");

    return state;
}

/* Network callbacks */

void
stomp_network_sent(struct stomp_state * state) {
    printf("stomp_network_sent: start.\n");

    printf("stomp_network_sent: frame has been sent.\n");
    stompc_sent();

    printf("stomp_network_sent: stop.\n");
}

void
stomp_network_received(struct stomp_state *state, char *buf, uint16_t len) {
    /* Potrzeba wykonac parsowanie strumienia znakow do ramki,
     * rozpoznac COMMAND ramki i w zaleznosci od COMMAND wykonac
     * operacje zgodnie ze specyfikacja protokolu. */

    /* Tutaj moze byc: CONNECTED (gdy CONNECT), MESSAGES, ERROR,
     * RECEIPT (gdy DISCONNECT). */

    printf("stomp_network_received: start.\n");

    printf("stomp_network_received: frame has been received: length=%d, content=%s\n", len, buf);
    stompc_received(buf, len);

    printf("stomp_network_received: stop.\n");

}

void
stomp_network_closed(struct stomp_state * state) {
    printf("stomp_network_closed: start.\n");

    printf("stomp_network_closed: closed.\n");

    printf("stomp_network_closed: stop.\n");
}

void
stomp_network_aborted(struct stomp_state * state) {
    printf("stomp_network_aborted: start.\n");

    printf("stomp_network_aborted: aborted.\n");

    printf("stomp_network_aborted: stop.\n");
}

void
stomp_network_timedout(struct stomp_state * state) {
    printf("stomp_network_timedout: start.\n");

    printf("stomp_network_timedout: timedout.\n");

    printf("stomp_network_timedout: stop.\n");
}