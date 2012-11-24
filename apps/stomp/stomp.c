#include "stomp-global.h"

#include "stomp.h"
#include "stompc.h"
#include "stomp-strings.h"
#include "stomp-frame.h"
#include "stomp-tools.h"

#include "uip-debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char stomp_version_default[4] = {0x31, 0x2e, 0x31,};

const char stomp_content_type_default[11] = {0x74, 0x65, 0x78, 0x74, 0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e,};

void
stomp_connect(char* host, char* login, char* passcode) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    PRINTA("stomp_connect: start.\n");
    PRINTA("CONNECT: host=%s, login=%s, pass=%s\n", host, login, passcode);
#endif

    headers = stomp_frame_new_header(stomp_header_accept_version, stomp_version_default);
    if (passcode != NULL) {
        headers = stomp_frame_add_header(stomp_header_passcode, passcode, headers);
    }
    if (login != NULL) {
        headers = stomp_frame_add_header(stomp_header_login, login, headers);
    }
    if (host != NULL) {
        headers = stomp_frame_add_header(stomp_header_host, host, headers);
    } else {
        PRINTA("stomp_connect: no host for CONNECT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_connect, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    PRINTA("stomp_connect: stop.\n");
#endif
}

void
stomp_subscribe(char *id, char *destination, char *ack) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    PRINTA("stomp_subscribe: start.\n");
    PRINTA("SUBSCRIBE: id=%s, destination=%s, ack=%s\n", id, destination, ack);
#endif

    if (ack != NULL) {
        headers = stomp_frame_new_header(stomp_header_ack, ack);
    } else {
        PRINTA("stomp_subscribe: no ack for SUBSCRIBE. Set to 'auto'.\n");
        headers = stomp_frame_new_header(stomp_header_ack, stomp_header_auto);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        PRINTA("stomp_subscribe: no destination for SUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }
    if (id != NULL) {
        headers = stomp_frame_add_header(stomp_header_id, id, headers);
    } else {
        PRINTA("stomp_subscribe: no id for SUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_subscribe, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    PRINTA("stomp_subscribe: stop.\n");
#endif
}

void
stomp_unsubscribe(char *id) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    PRINTA("UNSUBSCRIBE: id=%s\n", id);
#endif

    if (id != NULL) {
        headers = stomp_frame_new_header(stomp_header_id, id);
    } else {
        PRINTA("stomp_unsubscribe: no id for UNSUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_unsubscribe, headers, NULL);
    stompc_frame();
}

void
stomp_send(char *destination, char *type, char *length, char *receipt, char *tx, char *message) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    PRINTA("SEND: dest=%s, type=%s, len=%s, receipt=%s, tx=%s, msg=%s\n", destination, type, length, receipt, tx, message);
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    }
    if (receipt != NULL) {
        headers = stomp_frame_add_header(stomp_header_receipt, receipt, headers);
    }
    if (length != NULL) {
        headers = stomp_frame_add_header(stomp_header_content_length, length, headers);
    } else {
        char *_length = NEW_ARRAY(char, 3);
        PRINTA("stomp_send: no content-length for SEND. Set to computed value.\n");

        sprintf((char*) _length, "%u", (unsigned int) strlen((char*) message));
        headers = stomp_frame_add_header(stomp_header_content_length, _length, headers);
    }
    if (type != NULL) {
        headers = stomp_frame_add_header(stomp_header_content_type, type, headers);
    } else {
        PRINTA("stomp_send: no content-type for SEND. Set to 'text/plain'.\n");
        headers = stomp_frame_add_header(stomp_header_content_type, stomp_content_type_default, headers);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        PRINTA("stomp_send: no destination for SEND. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_send, headers, message);
    stompc_frame();
}

void
stomp_begin(char *tx) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    PRINTA("BEGIN: tx=%s\n", tx);
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        PRINTA("stomp_begin: no tx for BEGIN. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_begin, headers, NULL);
    stompc_frame();

}

void
stomp_commit(char *tx) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    PRINTA("COMMIT: tx=%s\n", tx);
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        PRINTA("stomp_commit: no tx for COMMIT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_commit, headers, NULL);
    stompc_frame();

}

void
stomp_abort(char *tx) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    PRINTA("ABORT: tx=%s\n", tx);
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        PRINTA("stomp_abort: no tx for ABORT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_abort, headers, NULL);
    stompc_frame();

}

void
stomp_disconnect(char *receipt) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    PRINTA("DISCONNECT: receipt=%s\n", receipt);
#endif

    if (receipt != NULL) {
        headers = stomp_frame_new_header(stomp_header_receipt, receipt);
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_disconnect, headers, NULL);
    stompc_frame();

}

/* Callbacks */

#ifndef WITH_UDP

void
stompc_connected() {
#ifdef STOMPC_TRACE
    PRINTA("CONNECTED:\n");
#endif
    stomp_connected();
}
#endif

#ifndef WITH_UDP

void
stompc_sent() {
#ifdef STOMPC_TRACE
    PRINTA("SENT:\n");
#endif
    stomp_sent();
}
#endif

void
stompc_received(char *buf, int len) {
    struct stomp_frame *frame;
#ifdef STOMPC_TRACE
    PRINTA("RECEIVED: len=%d, buf=%s\n", len, buf);
#endif
    frame = stomp_frame_import(buf, len, NULL);
    stomp_received(frame);
}

#ifndef WITH_UDP

void
stompc_closed() {
#ifdef STOMPC_TRACE
    PRINTA("CLOSED:\n");
#endif
    stomp_closed();
}
#endif