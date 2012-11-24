#include "simple-stomp.h"

#include "stomp-tools.h"
#include "stomp-frame.h"
#include "stomp-strings.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ISO_NULL 0x00
#define ISO_NL 0x0a
#define ISO_COLON 0x3a

char version[4] = {0x31, 0x2e, 0x31,};

char default_content_type[11] = {0x74, 0x65, 0x78, 0x74, 0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e,};

void
_simple_udp_send(struct simple_stomp_state *s) {
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_udp_send: start.\n");
#endif
    uip_udp_packet_sendto(s->conn, s->outputbuf, s->outputbuf_len, s->addr, UIP_HTONS(s->port));
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_udp_send: stop.\n");
#endif
}

static
PT_THREAD(_simple_tcp_send(struct simple_stomp_state *s)) {
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_tcp_send: start.\n");
#endif
    PSOCK_BEGIN(&s->s);
    PSOCK_SEND(&s->s, (uint8_t*) s->outputbuf, (unsigned int) s->outputbuf_len);
    PSOCK_END(&s->s);
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_tcp_send: stop.\n");
#endif
}

void simple_send(struct simple_stomp_state *s) {
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_send: start.\n");
#endif
    s->outputbuf_len = stomp_frame_length(s->frame);
    stomp_frame_export(s->frame, (char*) s->outputbuf, s->outputbuf_len);

    stomp_frame_delete_frame(s->frame);
    s->frame = NULL;

    PRINTA("%s\n", s->outputbuf);

#ifdef WITH_UDP
    _simple_udp_send(s);
#else
    _simple_tcp_send(s);
#endif
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_send: stop.\n");
#endif
}

static
PT_THREAD(simple_handle_connection(struct simple_stomp_state *s)) {
    PT_BEGIN(&s->pt);
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_handle_connection: start.\n");
#endif

    PSOCK_INIT(&s->s, (uint8_t*) s->inputbuf, sizeof (s->inputbuf) - 1);
    PSOCK_WAIT_UNTIL(&s->s, PSOCK_NEWDATA(&s->s) || (s->frame != NULL));

    if (PSOCK_NEWDATA(&s->s)) {
        PSOCK_READTO(&s->s, ISO_NULL);
        if (PSOCK_DATALEN(&s->s) > 0) {
            s->inputbuf[PSOCK_DATALEN(&s->s)] = 0;
            PRINTA("%s\n", s->inputbuf);
            /* TODO import frame */
        }
    }
    if (s->frame != NULL) {
        s->outputbuf_len = stomp_frame_length(s->frame);
        stomp_frame_export(s->frame, (char*) s->outputbuf, s->outputbuf_len);

        stomp_frame_delete_frame(s->frame);
        s->frame = NULL;

        PRINTA("%s\n", s->outputbuf);
        PSOCK_SEND(&s->s, (uint8_t*) s->outputbuf, (unsigned int) s->outputbuf_len);
    }

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_handle_connection: stop.\n");
#endif
    PT_END(&s->pt);
}

void
simple_app(void *s) {
    struct simple_stomp_state *state = (struct simple_stomp_state*) s;
#ifdef WITH_UDP
    char *str;
#endif

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_app: start.\n");
#endif

#ifdef WITH_UDP
    if (uip_newdata()) {
        str = uip_appdata;
        str[uip_datalen()] = '\0';
        PRINTA("%s\n", str);

    }
    if (s != NULL && state->frame != NULL) {
        simple_send(state);
    }
#else
    if (uip_aborted() || uip_timedout() || uip_closed()) {
        simple_disconnected(s);

    } else if (uip_connected()) {
        simple_connected(s);
        PT_INIT(&state->pt);
        simple_handle_connection(s);

    } else if (s != NULL) {
        simple_handle_connection(s);
    }
#endif

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_app: stop.\n");
#endif
}

struct simple_stomp_state *
simple_connect(struct simple_stomp_state *s, uip_ipaddr_t *addr, uint16_t port, char *host, char *login, char *pass) {
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_connect: start.\n");
#endif

#ifdef WITH_UDP
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("UDP connect\n");
#endif
    s->conn = udp_new(addr, UIP_HTONS(port), s);
#else
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("TCP connect\n");
#endif
    s->conn = tcp_connect(addr, UIP_HTONS(port), s);
#endif

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("Connecting...\n");
#endif

    if (s->conn == NULL) {
#ifdef SIMPLE_STOMP_TRACE
        PRINTA("Not connected.\n");
#endif
        return NULL;
    }

#ifdef WITH_UDP
    udp_bind(s->conn, UIP_HTONS(port + 1));
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("Binding...\n");
#endif
#endif

    s->addr = addr;
    s->port = port;

    s->host = host;
    s->login = login;
    s->pass = pass;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_connect: stop.\n");
#endif

    return s;
}

void
simple_connected(struct simple_stomp_state *s) {
    struct stomp_header *headers = NULL;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_connected: start.\n");
#endif

    headers = stomp_frame_new_header(stomp_header_accept_version, version);
    if (s->pass != NULL) {
        headers = stomp_frame_add_header(stomp_header_passcode, s->pass, headers);
    }
    if (s->login != NULL) {
        headers = stomp_frame_add_header(stomp_header_login, s->login, headers);
    }
    if (s->host != NULL) {
        headers = stomp_frame_add_header(stomp_header_host, s->host, headers);
    } else {
        PRINTA("No host for CONNECT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_connect, headers, NULL);

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_connected: stop.\n");
#endif
}

void
simple_stomp_subscribe(struct simple_stomp_state *s, char *id, char *destination, char *ack) {
    struct stomp_header *headers = NULL;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_subscribe: start.\n");
#endif

    if (ack != NULL) {
        headers = stomp_frame_new_header(stomp_header_ack, ack);
    } else {
        PRINTA("No ack for SUBSCRIBE. Set to 'auto'.\n");
        headers = stomp_frame_new_header(stomp_header_ack, stomp_header_auto);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        PRINTA("No destination for SUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }
    if (id != NULL) {
        headers = stomp_frame_add_header(stomp_header_id, id, headers);
    } else {
        PRINTA("No id for SUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_subscribe, headers, NULL);

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_subscribe: stop.\n");
#endif
}

void
simple_stomp_unsubscribe(struct simple_stomp_state *s, char *id) {
    struct stomp_header *headers = NULL;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_unsubscribe: start.\n");
#endif

    if (id != NULL) {
        headers = stomp_frame_new_header(stomp_header_id, id);
    } else {
        PRINTA("No id for UNSUBSCRIBE. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_unsubscribe, headers, NULL);

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_unsubscribe: stop.\n");
#endif
}

void
simple_stomp_send(struct simple_stomp_state *s, char *destination, char *type,
        char *length, char *receipt, char *tx, char *message) {
    struct stomp_header *headers = NULL;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_send: start.\n");
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
        PRINTA("No content-length for SEND. Set to computed value.\n");

        sPRINTA((char*) _length, "%u", (unsigned int) strlen((char*) message));
        headers = stomp_frame_add_header(stomp_header_content_length, _length, headers);
    }
    if (type != NULL) {
        headers = stomp_frame_add_header(stomp_header_content_type, type, headers);
    } else {
        PRINTA("No content-type for SEND. Set to 'text/plain'.\n");
        headers = stomp_frame_add_header(stomp_header_content_type, default_content_type, headers);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        PRINTA("No destination for SEND. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_send, headers, message);

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_send: stop.\n");
#endif
}

void
simple_stomp_begin(struct simple_stomp_state *s, char *tx) {
    struct stomp_header *headers = NULL;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_begin: start.\n");
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        PRINTA("No tx for BEGIN. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_begin, headers, NULL);

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_begin: stop.\n");
#endif
}

void
simple_stomp_commit(struct simple_stomp_state *s, char *tx) {
    struct stomp_header *headers = NULL;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_commit: start.\n");
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        PRINTA("No tx for COMMIT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_commit, headers, NULL);

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_commit: stop.\n");
#endif
}

void
simple_stomp_abort(struct simple_stomp_state *s, char *tx) {
    struct stomp_header *headers = NULL;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_abort: start.\n");
#endif

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        PRINTA("No tx for ABORT. Abort.\n");
        stomp_frame_delete_header(headers);
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_abort, headers, NULL);

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_abort: stop.\n");
#endif
}

void
simple_stomp_disconnect(struct simple_stomp_state *s, char *receipt) {
    struct stomp_header *headers = NULL;

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_disconnect: start.\n");
#endif

    if (receipt != NULL) {
        headers = stomp_frame_new_header(stomp_header_receipt, receipt);
    }

    s->frame = stomp_frame_new_frame(stomp_command_disconnect, headers, NULL);

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("simple_stomp_disconnect: stop.\n");
#endif
}

void
simple_disconnected(struct simple_stomp_state *s) {
#ifdef SIMPLE_STOMP_TRACE
    PRINTA("stomp_disconnected: start.\n");
#endif

    PRINTA("Disconnected.\n");

#ifdef SIMPLE_STOMP_TRACE
    PRINTA("stomp_disconnected: stop.\n");
#endif
}