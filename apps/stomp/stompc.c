#include "stomp-global.h"

#include "stompc.h"
#include "stomp-network.h"
#include "stomp-frame.h"
#include "stomp-tools.h"

#include "uip-debug.h"

#include <stdio.h>

struct stompc_state c_state;

void
stompc_frame() {
    int len;
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

#ifndef WITH_UDP

void
stomp_network_connected() {
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_connected: connected.\n");
#endif    
    stompc_connected();
}
#endif

#ifndef WITH_UDP

void
stomp_network_sent() {
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_sent: frame has been sent.\n");
#endif    
    stompc_sent();
}
#endif

void
stomp_network_received(char *buf, int len) {
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_received: frame has been received.\n");
#endif    
    stompc_received(buf, len);
}

#ifndef WITH_UDP

void
stomp_network_closed() {
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_closed: closed.\n");
#endif
    stompc_closed();
}
#endif

#ifndef WITH_UDP

void
stomp_network_aborted() {
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_aborted: aborted.\n");
#endif
}
#endif

#ifndef WITH_UDP

void
stomp_network_timedout() {
#ifdef STOMP_NETWORK_TRACE
    PRINTA("stomp_network_timedout: timedout.\n");
#endif
}
#endif