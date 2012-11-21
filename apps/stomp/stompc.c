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
    PROCESS_BEGIN();
#ifdef STOMPC_TRACE
    printf("stompc_process: start.\n");
#endif

    while (1) {
        PROCESS_WAIT_EVENT();
#ifdef STOMPC_TRACE
        printf("stompc_process: any event.\n");
#endif
        stompc_frame();
    }
#ifdef STOMPC_TRACE
    printf("stompc_process: stop.\n");
#endif
    PROCESS_END();
}

void
stompc_frame() {
    uint16_t len;
    char *buf;

    if (c_state.frame != NULL) {
        len = stomp_frame_length(c_state.frame);
        buf = NEW_ARRAY(char, len);
        stomp_frame_export(c_state.frame, buf, len);

        stomp_frame_delete_frame(c_state.frame);
        c_state.frame = NULL;

        stomp_network_send(buf, len);
    }
}

/* Callbacks */

void
stomp_network_connected() {
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_connected: start.\n");
    printf("stomp_network_connected: connected.\n");
#endif    
    stompc_connected();
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_connected: stop.\n");
#endif
}

void
stomp_network_sent() {
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_sent: start.\n");
    printf("stomp_network_sent: frame has been sent.\n");
#endif    
    stompc_sent();
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_sent: stop.\n");
#endif
}

void
stomp_network_received(char *buf, uint16_t len) {
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_received: start.\n");
#endif    
    stompc_received(buf, len);
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_received: stop.\n");
#endif
}

void
stomp_network_closed() {
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_closed: start.\n");
    printf("stomp_network_closed: closed.\n");
#endif
    stompc_closed();
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_closed: stop.\n");
#endif
}

void
stomp_network_aborted() {
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_aborted: start.\n");
    printf("stomp_network_aborted: aborted.\n");
#endif
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_aborted: stop.\n");
#endif
}

void
stomp_network_timedout() {
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_timedout: start.\n");
    printf("stomp_network_timedout: timedout.\n");
#endif
#ifdef STOMP_NETWORK_TRACE
    printf("stomp_network_timedout: stop.\n");
#endif
}