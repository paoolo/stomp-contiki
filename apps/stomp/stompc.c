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

const char stomp_version_default[4] =
/* "1.1" */
{0x31,0x2e,0x31,};

const char stomp_content_type_default[11] =
/* "plain/text" */
{0x74,0x65,0x78,0x74,0x2f,0x70,0x6c,0x61,0x69,0x6e,};

struct stomp_state state;

PROCESS(stompc_process, "StompC");

PROCESS_THREAD(stompc_process, ev, data)
{
    PROCESS_BEGIN();

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
        printf("TCP/IP event.\n");
        stomp_network_app(data);
    }

    PROCESS_END();

}

struct stomp_state *
stompc_connect(struct stomp_state *state, uip_ipaddr_t *addr, uint16_t port, char *host, char *login, char *passcode)
{
    if (stomp_network_connect(state, addr, port) == NULL) {
        return NULL;
    }

    state->host = host;
    state->login = login;
    state->passcode = passcode;

    return state;
}

void stomp_network_connected(struct stomp_state *state)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    printf("Connected.\n");

    headers = stomp_frame_new_header(stomp_header_accept_version, stomp_version_default);
    if (state->passcode != NULL) {
        headers = stomp_frame_add_header(stomp_header_passcode, state->passcode, headers);
    }
    if (state->login != NULL) {
        headers = stomp_frame_add_header(stomp_header_login, state->login, headers);
    }
    if (state->host != NULL) {
        headers = stomp_frame_add_header(stomp_header_host, state->host, headers);
    }

    frame = stomp_frame_new_frame(stomp_command_connect, headers, NULL);

    len = stomp_frame_length(frame);
    buf = NEW_ARRAY(char, len);

    stomp_frame_export(frame, buf, len);
    stomp_frame_delete_frame(frame);

    stomp_network_send(state, buf, len);
}

void stomp_network_sent(struct stomp_state *state)
{
    printf("Frame has been sent.\n");
}

void stomp_network_received(struct stomp_state *state, char *buf, uint16_t len)
{
    /* Potrzeba wykonac parsowanie strumienia znakow do ramki,
     * rozpoznac COMMAND ramki i w zaleznosci od COMMAND wykonac
     * operacje zgodnie ze specyfikacja protokolu. */

    /* Tutaj moze byc: CONNECTED (gdy CONNECT), MESSAGES, ERROR,
     * RECEIPT (gdy DISCONNECT). */

    printf("Frame has been received: length=%d, content=%s\n", len, buf);
}

void
stompc_subscribe(struct stomp_state *state, char *id, char *destination, char *ack)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    if (ack != NULL) {
        headers = stomp_frame_new_header(stomp_header_ack, ack);
    } else {
        printf("No ack for SUBSCRIBE. Set to 'auto'.\n");
        headers = stomp_frame_new_header(stomp_header_ack, stomp_header_auto);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        printf("No destination for SUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }
    if (id != NULL) {
        headers = stomp_frame_add_header(stomp_header_id, id, headers);
    } else {
        printf("No id for SUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    frame = stomp_frame_new_frame(stomp_command_subscribe, headers, NULL);

    len = stomp_frame_length(frame);
    buf = NEW_ARRAY(char, len);

    stomp_frame_export(frame, buf, len);
    stomp_frame_delete_frame(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_unsubscribe(struct stomp_state *state, char *id)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    if (id != NULL) {
        headers = stomp_frame_new_header(stomp_header_id, id);
    } else {
        printf("No id for UNSUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    frame = stomp_frame_new_frame(stomp_command_unsubscribe, headers, NULL);

    len = stomp_frame_length(frame);
    buf = NEW_ARRAY(char, len);

    stomp_frame_export(frame, buf, len);
    stomp_frame_delete_frame(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_send(struct stomp_state *state, char *destination, char *type, char *length, char *receipt, char *tx, char *message)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    }
    if (receipt != NULL) {
        headers = stomp_frame_add_header(stomp_header_receipt, receipt, headers);
    }
    if (length != NULL) {
        headers = stomp_frame_add_header(stomp_header_content_length, length, headers);
    } else {
        printf("No content-length for SEND. Set to computed value.\n");
        length = NEW_ARRAY(char, 3); sprintf(length, "%u", (unsigned int)strlen(message));
        headers = stomp_frame_add_header(stomp_header_content_length, length, headers);
    }
    if (type != NULL) {
        headers = stomp_frame_add_header(stomp_header_content_type, type, headers);
    } else {
        printf("No content-type for SEND. Set to 'text/plain'.\n");
        headers = stomp_frame_add_header(stomp_header_content_type, stomp_content_type_default, headers);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        printf("No destination for SEND. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    frame = stomp_frame_new_frame(stomp_command_send, headers, message);

    len = stomp_frame_length(frame);
    buf = NEW_ARRAY(char, len);

    stomp_frame_export(frame, buf, len);
    stomp_frame_delete_frame(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_begin(struct stomp_state *state, char *tx)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("No tx for BEGIN. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }
    frame = stomp_frame_new_frame(stomp_command_begin, headers, NULL);

    len = stomp_frame_length(frame);
    buf = NEW_ARRAY(char, len);

    stomp_frame_export(frame, buf, len);
    stomp_frame_delete_frame(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_commit(struct stomp_state *state, char *tx)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("No tx for COMMIT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }
    frame = stomp_frame_new_frame(stomp_command_commit, headers, NULL);

    len = stomp_frame_length(frame);
    buf = NEW_ARRAY(char, len);

    stomp_frame_export(frame, buf, len);
    stomp_frame_delete_frame(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_abort(struct stomp_state *state, char *tx)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("No tx for ABORT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }
    frame = stomp_frame_new_frame(stomp_command_abort, headers, NULL);

    len = stomp_frame_length(frame);
    buf = NEW_ARRAY(char, len);

    stomp_frame_export(frame, buf, len);
    stomp_frame_delete_frame(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_disconnect(struct stomp_state *state, char *receipt)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    headers = stomp_frame_new_header(stomp_header_receipt, receipt);
    frame = stomp_frame_new_frame(stomp_command_disconnect, headers, NULL);

    len = stomp_frame_length(frame);
    buf = NEW_ARRAY(char, len);

    stomp_frame_export(frame, buf, len);
    stomp_frame_delete_frame(frame);

    stomp_network_send(state, buf, len);
}

void
stomp_network_closed(struct stomp_state *state)
{
    printf("Closed.\n");
}

void
stomp_network_aborted(struct stomp_state *state)
{
    printf("Aborted.\n");
}

void
stomp_network_timedout(struct stomp_state *state)
{
    printf("Timedout.\n");
}