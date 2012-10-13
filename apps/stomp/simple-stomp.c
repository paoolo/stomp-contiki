#include "contiki.h"
#include "contiki-net.h"

#include "simple-stomp.h"

#include "stomp-frame.h"
#include "stomp-strings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SEND_STRING(s, str, len) PSOCK_SEND(s, (uint8_t *)str, (unsigned int)len)

#define ISO_NULL 0x00
#define ISO_NL 0x0a
#define ISO_COLON 0x3a

const char version[4] = {0x31,0x2e,0x31,};

const char default_content_type[11] = {0x74,0x65,0x78,0x74,0x2f,0x70,0x6c,0x61,0x69,0x6e,};

struct simple_stomp_state simple_state;

static
PT_THREAD(simple_send(struct simple_stomp_state *s))
{
    char *buf = NULL;
    unsigned int len = 0;
    
    PSOCK_BEGIN(&s->s);
    
    buf = stomp_frame_export(s->frame);
    len = stomp_frame_length(s->frame);

    printf("Frame: len=%d body=%s\n", len, buf);
    
    SEND_STRING(&s->s, buf, len);
    
    PSOCK_END(&s->s);
}

static
PT_THREAD(simple_handle_input(struct simple_stomp_state *s))
{
    PSOCK_BEGIN(&s->s);

    PSOCK_READTO(&s->s, ISO_NULL);
    
    if(PSOCK_DATALEN(&s->s) > 0) {
        s->inputbuf[PSOCK_DATALEN(&s->s)] = 0;
        printf("%s\n", s->inputbuf);
        /* TODO import frame*/
    }
    
    PSOCK_END(&s->s);
}

static
PT_THREAD(simple_data_or_command(struct simple_stomp_state *s))
{
    PSOCK_BEGIN(&s->s);

    PSOCK_WAIT_UNTIL(&s->s, PSOCK_NEWDATA(&s->s) || (s->frame != NULL));

    PSOCK_END(&s->s);
}

static
PT_THREAD(simple_handle_connection(struct simple_stomp_state *s))
{
    PT_BEGIN(&s->pt);
    
    PSOCK_INIT(&s->s, (uint8_t*)s->inputbuf, sizeof(s->inputbuf) - 1);
    
    while(1) {
        PT_WAIT_UNTIL(&s->pt, simple_data_or_command(s));
        
        if(PSOCK_NEWDATA(&s->s)) {
            PT_WAIT_THREAD(&s->pt, simple_handle_input(s));      
        }
        
        if (s->frame != NULL) {
            PT_WAIT_THREAD(&s->pt, simple_send(s));
        }
    }
    
    PT_END(&s->pt);
}

void
simple_app(void *s)
{
    struct simple_stomp_state *state = (struct simple_stomp_state*)s;
    
    if(uip_closed() || uip_aborted() || uip_timedout()) {
        simple_disconnected(state);
        
    } else if(uip_connected()) {
        simple_connected(s);
        PT_INIT(&(state->pt));
        simple_handle_connection(state);
        
    } else if(s != NULL) {
        simple_handle_connection(state);
    }
}

struct simple_stomp_state *
simple_connect(struct simple_stomp_state *s, uip_ipaddr_t *ipaddr, uint16_t port, char *host, char *login, char *pass)
{
    s->conn = tcp_connect((uip_ipaddr_t*)ipaddr, UIP_HTONS(port), s);
    if(s->conn == NULL) {
        printf("Not connected.\n");
        return NULL;
    }
    s->host = host;
    s->login = login;
    s->pass = pass;
    return s;
}

void
simple_connected(struct simple_stomp_state *s)
{
    struct stomp_header *headers = NULL;
    
    printf("Connected.\n");
    
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
        printf("No host for CONNECT. Abort.\n");
        return;
    }
    
    s->frame = stomp_frame_new_frame(stomp_command_connect, headers, NULL);
    printf("%s\n", stomp_frame_export(s->frame));
}

void
simple_stomp_subscribe(struct simple_stomp_state *s, char *id, char *destination, char *ack)
{
    struct stomp_header *headers = NULL;

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
        return;
    }
    if (id != NULL) {
        headers = stomp_frame_add_header(stomp_header_id, id, headers);
    } else {
        printf("No id for SUBSCRIBE. Abort.\n");
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_subscribe, headers, NULL);
    printf("%s\n", stomp_frame_export(s->frame));
}

void
simple_stomp_unsubscribe(struct simple_stomp_state *s, char *id)
{
    struct stomp_header *headers = NULL;
    
    if (id != NULL) {
        headers = stomp_frame_new_header(stomp_header_id, id);
    } else {
        printf("No id for UNSUBSCRIBE. Abort.\n");
        return;
    }

    s->frame = stomp_frame_new_frame(stomp_command_unsubscribe, headers, NULL);
    printf("%s\n", stomp_frame_export(s->frame));
}

void
simple_stomp_send(struct simple_stomp_state *s, char *destination, char *type, char *length, char *receipt, char *tx, char *message)
{
    struct stomp_header *headers = NULL;
    
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
        /* TODO */
    }
    if (type != NULL) {
        headers = stomp_frame_add_header(stomp_header_content_type, type, headers);
    } else {
        printf("No content-type for SEND. Set to 'text/plain'.\n");
        headers = stomp_frame_add_header(stomp_header_content_type, default_content_type, headers);
    }
    if (destination != NULL) {
        headers = stomp_frame_add_header(stomp_header_destination, destination, headers);
    } else {
        printf("No destination for SEND. Abort.\n");
        return;
    }
    
    s->frame = stomp_frame_new_frame(stomp_command_send, headers, message);
    printf("%s\n", stomp_frame_export(s->frame));
}

void
simple_stomp_begin(struct simple_stomp_state *s, char *tx)
{
    struct stomp_header *headers = NULL;

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("No tx for BEGIN. Abort.\n");
        return;
    }
    
    s->frame = stomp_frame_new_frame(stomp_command_begin, headers, NULL);
    printf("%s\n", stomp_frame_export(s->frame));
}

void
simple_stomp_commit(struct simple_stomp_state *s, char *tx)
{
    struct stomp_header *headers = NULL;

    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("No tx for COMMIT. Abort.\n");
        return;
    }
    
    s->frame = stomp_frame_new_frame(stomp_command_commit, headers, NULL);
    printf("%s\n", stomp_frame_export(s->frame));
}

void
simple_stomp_abort(struct simple_stomp_state *s, char *tx)
{
    struct stomp_header *headers = NULL;
    
    if (tx != NULL) {
        headers = stomp_frame_new_header(stomp_header_transaction, tx);
    } else {
        printf("No tx for ABORT. Abort.\n");
        return;
    }
    
    s->frame = stomp_frame_new_frame(stomp_command_abort, headers, NULL);
    printf("%s\n", stomp_frame_export(s->frame));
}

void
simple_stomp_disconnect(struct simple_stomp_state *s, char *receipt)
{
    struct stomp_header *headers = NULL;
    
    if (receipt != NULL) {
        headers = stomp_frame_new_header(stomp_header_receipt, receipt);
    }
    
    s->frame = stomp_frame_new_frame(stomp_command_disconnect, headers, NULL);
    printf("%s\n", stomp_frame_export(s->frame));
}

void
simple_disconnected(struct simple_stomp_state *s)
{
    printf("Disconnected.\n");
}