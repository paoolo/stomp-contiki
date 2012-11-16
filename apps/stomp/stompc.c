#include "stompc.h"

#include "contiki.h"
#include "contiki-net.h"

#include "stomp.h"
#include "stomp-tools.h"
#include "stomp-frame.h"
#include "stomp-network.h"
#include "stomp-strings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct stompc_state c_state;

PROCESS(stompc_process, "STOMPc process");
AUTOSTART_PROCESSES(&stompc_process);

PROCESS_THREAD(stompc_process, ev, data) {
    uint16_t len;
    char *buf;

    PROCESS_BEGIN();
    printf("stompc_process: start.\n");

    while (1) {
        PROCESS_WAIT_EVENT();
        printf("stompc_process: any event.\n");
        if (c_state.frame != NULL) {
            len = stomp_frame_length(c_state.frame);
            buf = NEW_ARRAY(char, len);
            stomp_frame_export(c_state.frame, buf, len);

            stomp_frame_delete_frame(c_state.frame);
            c_state.frame = NULL;

            stomp_network_send(buf, len);
            process_post(&stomp_network_process, PROCESS_EVENT_CONTINUE, NULL);
        }
    }

    printf("stompc_process: stop.\n");
    PROCESS_END();
}

void
stomp_network_connected() {
    printf("stomp_network_connected: start.\n");

    printf("stomp_network_connected: connected.\n");
    stompc_connected();

    printf("stomp_network_connected: stop.\n");
}

void
stomp_network_sent() {
    printf("stomp_network_sent: start.\n");

    printf("stomp_network_sent: frame has been sent.\n");
    stompc_sent();

    printf("stomp_network_sent: stop.\n");
}

void
stomp_network_received(char *buf, uint16_t len) {
    printf("stomp_network_received: start.\n");

    printf("stomp_network_received: frame has been received: length=%d, content=%s\n", len, buf);
    stompc_received(buf, len);

    printf("stomp_network_received: stop.\n");
}

void
stomp_network_closed() {
    printf("stomp_network_closed: start.\n");

    printf("stomp_network_closed: closed.\n");
    stompc_closed();

    printf("stomp_network_closed: stop.\n");
}

void
stomp_network_aborted() {
    printf("stomp_network_aborted: start.\n");

    printf("stomp_network_aborted: aborted.\n");

    printf("stomp_network_aborted: stop.\n");
}

void
stomp_network_timedout() {
    printf("stomp_network_timedout: start.\n");

    printf("stomp_network_timedout: timedout.\n");

    printf("stomp_network_timedout: stop.\n");
}