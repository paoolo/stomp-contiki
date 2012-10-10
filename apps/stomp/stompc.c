#include "stompc.h"

#include "contiki.h"
#include "contiki-net.h"

#include "stomp-network.h"
#include "stomp-strings.h"

static struct stomp_state state;

PROCESS(stomp_process, "Stomp client");

AUTOSTART_PROCESSES(&stomp_process);

PROCESS_THREAD(stomp_process, ev, data)
{
    PROCESS_BEGIN();

    while(1) {
        PROCESS_WAIT_EVENT();

        if(ev == PROCESS_EVENT_EXIT){
            printf("Exit.\n");
            process_exit(&stomp_process);
            LOADER_UNLOAD();

        } else if(ev == tcpip_event) {
            printf("Networking.\n");
            stomp_network_app(data);
        }
    }

    PROCESS_END();

}

struct stomp_state *
stompc_connect(struct stomp_state *state, uip_ipaddr_t *addr, uint16_t port, char *host, char *login, char *passcode)
{
    if (stomp_network_connect(&state->network_state, addr, port) == NULL) {
        return NULL;
    }
    return state;
}

void stomp_network_connected(struct stomp_network_state *state)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    printf("Connected.\n");

    if (passcode != NULL && login != NULL) {
        headers = stomp_frame_new_header(stomp_header_passcode, passcode);
        headers = stomp_frame_add_header(stomp_header_login, login, headers);
        headers = stomp_frame_add_header(stomp_header_host, host, headers);
    } else {
        headers = stomp_frame_new_header(stomp_header_host, host);
    }
    frame = stomp_frame_new_frame(stomp_command_connect, headers, NULL);

    buf = stomp_frame_export(frame);
    len = stomp_frame_length(frame);

    stomp_network_send(state, buffer, length);
}

void stomp_network_sent(struct stomp_network_state *state)
{
    printf("Frame has been sent.\n");
}

void stomp_network_received(struct stomp_network_state *state, char *buf, uint16_t len)
{
    /* Potrzeba wykonac parsowanie strumienia znakow do ramki,
     * rozpoznac COMMAND ramki i w zaleznosci od COMMAND wykonac
     * operacje zgodnie ze specyfikacja protokolu. */

    /* Tutaj moze byc: CONNECTED (gdy CONNECT), MESSAGES, ERROR,
     * RECEIPT (gdy DISCONNECT). */

    struct stomp_frame *frame = NULL;

    /* TODO */
    
    printf("%d\n", len);
    printf("%s\n", buf);
}

void
stompc_subscribe(struct stomp_state *state, char *client_id, char *destination)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    headers = stomp_frame_new_header(stomp_header_destination, destination);
    headers = stomp_frame_add_header(stomp_header_client_id, client_id, headers);
    frame = stomp_frame_new_frame(stomp_command_subscribe, headers, NULL);

    buf = stomp_frame_export(frame);
    len = stomp_frame_length(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_unsubscribe(struct stomp_state *state, char *client_id)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    headers = stomp_frame_new_header(stomp_header_client_id, client_id);
    frame = stomp_frame_new_frame(stomp_command_unsubscribe, headers, NULL);

    buf = stomp_frame_export(frame);
    len = stomp_frame_length(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_send(struct stomp_state *state, char *destination, char *tx, char *message)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        headers = stomp_frame_new_header(stomp_header_destination, destination);
    }
    frame = stomp_frame_new_frame(stomp_command_send, headers, message);

    buf = stomp_frame_export(frame);
    len = stomp_frame_length(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_begin(struct stomp_state *state, char *tx)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    headers = stomp_frame_new_header(stomp_header_transaction, tx);
    frame = stomp_frame_new_frame(stomp_command_begin, headers, NULL);

    buf = stomp_frame_export(frame);
    len = stomp_frame_length(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_commit(struct stomp_state *state, char *tx)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    headers = stomp_frame_new_header(stomp_header_transaction, tx);
    frame = stomp_frame_new_frame(stomp_command_commit, headers, NULL);

    buf = stomp_frame_export(frame);
    len = stomp_frame_length(frame);

    stomp_network_send(state, buf, len);
}

void
stompc_abort(struct stomp_state *state, char *tx)
{
    char *buf = NULL;
    uint16_t len = 0;

    struct stomp_header *headers = NULL;
    struct stomp_frame *frame = NULL;

    headers = stomp_frame_new_header(stomp_header_transaction, tx);
    frame = stomp_frame_new_frame(stomp_command_abort, headers, NULL);

    buf = stomp_frame_export(frame);
    len = stomp_frame_length(frame);

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

    buf = stomp_frame_export(frame);
    len = stomp_frame_length(frame);

    stomp_network_send(state, buf, len);
}

void
stomp_network_closed(struct stomp_network_state *state)
{
    printf("Closed.\n");
}

void
stomp_network_aborted(struct stomp_network_state *state)
{
    printf("Aborted.\n");
}

void
stomp_network_timedout(struct stomp_network_state *state)
{
    printf("Timedout.\n");
}