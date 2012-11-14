#include "stompc.h"
#include "stomp-tools.h"

#include "contiki-net.h"
#include "process.h"

#include <stdlib.h>
#include <stdio.h>

void
stomp_app() {
    printf("APP:\n");
    stompc_app(&state);
}

void
stomp_connect(uip_ipaddr_t *addr, uint16_t port, char* host, char* login, char* passcode) {
    struct stomp_header *headers = NULL;

    printf("stomp_connect: start.\n");
    printf("CONNECT: host=%s, login=%s, pass=%s\n", host, login, passcode);

    state->network_state.addr = addr;
    state->network_state.port = port;

    headers = stomp_frame_new_header(stomp_header_accept_version, stomp_version_default);
    if (state->pass != NULL) {
        headers = stomp_frame_add_header(stomp_header_passcode, state->pass, headers);
    }
    if (state->login != NULL) {
        headers = stomp_frame_add_header(stomp_header_login, state->login, headers);
    }
    if (state->host != NULL) {
        headers = stomp_frame_add_header(stomp_header_host, state->host, headers);
    } else {
        printf("stomp_connect: no host for CONNECT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    state->frame = stomp_frame_new_frame(stomp_command_connect, headers, NULL);
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stomp_connect: start.\n");
}

void
stomp_subscribe(char *id, char *destination, char *ack) {
    struct stomp_header *headers = NULL;

    printf("stomp_subscribe: start.\n");
    printf("SUBSCRIBE: id=%s, destination=%s, ack=%s\n", id, destination, ack);

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

    state->frame = stomp_frame_new_frame(stomp_command_subscribe, headers, NULL);
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stomp_subscribe: stop.\n");
}

void
stomp_unsubscribe(char *id) {
    struct stomp_header *headers = NULL;

    printf("stomp_unsubscribe: start.\n");
    printf("UNSUBSCRIBE: id=%s\n", id);

    if (id != NULL) {
        headers = stomp_frame_new_header(stomp_header_id, id);
    } else {
        printf("stomp_unsubscribe: no id for UNSUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    state->frame = stomp_frame_new_frame(stomp_command_unsubscribe, headers, NULL);
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stomp_unsubscribe: start.\n");
}

void
stomp_send(char *destination, char *type, char *length, char *receipt, char *tx, char *message) {
    struct stomp_header *headers = NULL;

    printf("stomp_send: start.\n");
    printf("SEND: dest=%s, type=%s, len=%s, receipt=%s, tx=%s, msg=%s\n", destination, type, length, receipt, tx, message);

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

    state->frame = stomp_frame_new_frame(stomp_command_send, headers, message);
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stomp_send: stop.\n");
}

void
stomp_begin(char *tx) {
    struct stomp_header *headers = NULL;

    printf("stomp_begin: start.\n");
    printf("BEGIN: tx=%s\n", tx);

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("stomp_begin: no tx for BEGIN. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    state->frame = stomp_frame_new_frame(stomp_command_begin, headers, NULL);
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stomp_begin: stop.\n");
}

void
stomp_commit(char *tx) {
    struct stomp_header *headers = NULL;

    printf("stomp_commit: start.\n");
    printf("COMMIT: tx=%s\n", tx);

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("stomp_commit: no tx for COMMIT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    state->frame = stomp_frame_new_frame(stomp_command_commit, headers, NULL);
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stomp_commit: stop.\n");
}

void
stomp_abort(char *tx) {
    struct stomp_header *headers = NULL;

    printf("stomp_abort: start.\n");
    printf("ABORT: tx=%s\n", tx);

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("stomp_abort: no tx for ABORT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    state->frame = stomp_frame_new_frame(stomp_command_abort, headers, NULL);
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stomp_abort: stop.\n");
}

void
stomp_disconnect(char *receipt) {
    struct stomp_header *headers = NULL;

    printf("stomp_disconnect: start.\n");
    printf("DISCONNECT: receipt=%s\n", receipt);

    if (receipt != NULL) {
        headers = stomp_frame_new_header(stomp_header_receipt, receipt);
    }

    state->frame = stomp_frame_new_frame(stomp_command_disconnect, headers, NULL);
    process_post(&stompc_process, PROCESS_EVENT_CONTINUE, NULL);

    printf("stomp_disconnect: stop.\n");
}

void
stompc_sent() {
    printf("SENT:\n");
    stomp_sent();
}

void
stompc_received(char *buf, uint16_t len) {
    printf("RECEIVED: len=%d, buf=%s\n", len, buf);
    stomp_received(buf, len);
}