#include "stomp.h"

#include "stompc.h"
#include "stomp-tools.h"
#include "stomp-frame.h"
#include "stomp-strings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

const char stomp_version_default[4] = {0x31, 0x2e, 0x31,};

const char stomp_content_type_default[11] = {0x74, 0x65, 0x78, 0x74, 0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e,};

void
stomp_connect(char* host, char* login, char* passcode) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    printf("stomp_connect: start.\n");
    printf("CONNECT: host=%s, login=%s, pass=%s\n", host, login, passcode);
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
        printf("stomp_connect: no host for CONNECT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_connect, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    printf("stomp_connect: stop.\n");
#endif
}

void
stomp_subscribe(char *id, char *destination, char *ack) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    printf("stomp_subscribe: start.\n");
    printf("SUBSCRIBE: id=%s, destination=%s, ack=%s\n", id, destination, ack);
#endif

    if (ack != NULL) {
        headers = stomp_frame_new_header(stomp_header_ack, ack);
    } else {
        printf("stomp_subscribe: no ack for SUBSCRIBE. Set to 'auto'.\n");
        headers = stomp_frame_new_header(stomp_header_ack, stomp_header_auto);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        printf("stomp_subscribe: no destination for SUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }
    if (id != NULL) {
        headers = stomp_frame_add_header(stomp_header_id, id, headers);
    } else {
        printf("stomp_subscribe: no id for SUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_subscribe, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    printf("stomp_subscribe: stop.\n");
#endif
}

void
stomp_unsubscribe(char *id) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    printf("stomp_unsubscribe: start.\n");
    printf("UNSUBSCRIBE: id=%s\n", id);
#endif

    if (id != NULL) {
        headers = stomp_frame_new_header(stomp_header_id, id);
    } else {
        printf("stomp_unsubscribe: no id for UNSUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_unsubscribe, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    printf("stomp_unsubscribe: start.\n");
#endif
}

void
stomp_send(char *destination, char *type, char *length, char *receipt, char *tx, char *message) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    printf("stomp_send: start.\n");
    printf("SEND: dest=%s, type=%s, len=%s, receipt=%s, tx=%s, msg=%s\n", destination, type, length, receipt, tx, message);
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
        printf("stomp_send: no content-length for SEND. Set to computed value.\n");

        sprintf((char*) _length, "%u", (unsigned int) strlen((char*) message));
        headers = stomp_frame_add_header(stomp_header_content_length, _length, headers);
    }
    if (type != NULL) {
        headers = stomp_frame_add_header(stomp_header_content_type, type, headers);
    } else {
        printf("stomp_send: no content-type for SEND. Set to 'text/plain'.\n");
        headers = stomp_frame_add_header(stomp_header_content_type, stomp_content_type_default, headers);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        printf("stomp_send: no destination for SEND. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_send, headers, message);
    stompc_frame();

#ifdef STOMP_TRACE
    printf("stomp_send: stop.\n");
#endif
}

void
stomp_begin(char *tx) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    printf("stomp_begin: start.\n");
    printf("BEGIN: tx=%s\n", tx);
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("stomp_begin: no tx for BEGIN. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_begin, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    printf("stomp_begin: stop.\n");
#endif
}

void
stomp_commit(char *tx) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    printf("stomp_commit: start.\n");
    printf("COMMIT: tx=%s\n", tx);
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("stomp_commit: no tx for COMMIT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_commit, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    printf("stomp_commit: stop.\n");
#endif
}

void
stomp_abort(char *tx) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    printf("stomp_abort: start.\n");
    printf("ABORT: tx=%s\n", tx);
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("stomp_abort: no tx for ABORT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_abort, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    printf("stomp_abort: stop.\n");
#endif
}

void
stomp_disconnect(char *receipt) {
    struct stomp_header *headers = NULL;

#ifdef STOMP_TRACE
    printf("stomp_disconnect: start.\n");
    printf("DISCONNECT: receipt=%s\n", receipt);
#endif

    if (receipt != NULL) {
        headers = stomp_frame_new_header(stomp_header_receipt, receipt);
    }

    c_state.frame = stomp_frame_new_frame(stomp_command_disconnect, headers, NULL);
    stompc_frame();

#ifdef STOMP_TRACE
    printf("stomp_disconnect: stop.\n");
#endif
}

/* Callbacks */

void
stompc_connected() {
#ifdef STOMPC_TRACE
    printf("stompc_connected: start.\n");
    printf("CONNECTED:\n");
#endif
    stomp_connected();
#ifdef STOMPC_TRACE
    printf("stompc_connected: stop.\n");
#endif
}

void
stompc_sent() {
#ifdef STOMPC_TRACE
    printf("stompc_sent: start.\n");
    printf("SENT:\n");
#endif
    stomp_sent();
#ifdef STOMPC_TRACE
    printf("stompc_send: stop.\n");
#endif
}

void
stompc_received(char *buf, uint16_t len) {
#ifdef STOMPC_TRACE
    printf("stompc_received: starte.\n");
    printf("RECEIVED: len=%d, buf=%s\n", len, buf);
#endif
    stomp_received(NULL);
#ifdef STOMPC_TRACE
    printf("stompc_received: stop.\n");
#endif
}

void
stompc_closed() {
#ifdef STOMPC_TRACE
    printf("stompc_closed: start.\n");
    printf("CLOSED:\n");
#endif
    stomp_closed();
#ifdef STOMPC_TRACE
    printf("stompc_closed: stop.\n");
#endif
}