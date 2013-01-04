#include <string.h>

#include "stomp.h"
#include "stomp-strings.h"
#include "stomp-tools.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"
#include "stomp-network.h"

#define BUFFER_SIZE 256

#if UIP_CONF_IPV6 > 0
int addr[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
#else
int addr[] = {10, 1, 1, 100};
#endif

uip_ipaddr_t ipaddr;
int port = 61613;

PROCESS(stomp_test_process, "STOMP test");
AUTOSTART_PROCESSES(&stomp_test_process, &stomp_network_process);

static void
_stomp_sent(char *buf, int len) {
    PRINTA("Sent frame: {buf=\"%s\", len=\"%d\"}\n", buf, len);
    process_post(&stomp_test_process, PROCESS_EVENT_CONTINUE, NULL);
}

static void
_stomp_received(char *buf, int len) {
    PRINTA("Received frame: {buf=\"%s\", len=\"%d\"}\n", buf, len);
}

static void
_stomp_message_auto(char* destination, char* message_id, char* subscription, char* content_type, char* content_length, char* message) {
    PRINTA("MESSAGE: {destination=\"%s\", message_id=\"%s\", subscription=\"%s\", content_type=\"%s\", content_length=\"%s\", message=\"%s\"}.\n",
            destination, message_id, subscription, content_type, content_length, message);
}

static void
_stomp_message_client_individual(char* destination, char* message_id, char* subscription, char* content_type, char* content_length, char* message) {
    PRINTA("MESSAGE: {destination=\"%s\", message_id=\"%s\", subscription=\"%s\", content_type=\"%s\", content_length=\"%s\", message=\"%s\"}.\n",
            destination, message_id, subscription, content_type, content_length, message);
    stomp_ack(subscription, message_id, NULL);
}

static void
_stomp_error(char* receipt_id, char* content_type, char* content_length, char *message) {
    PRINTA("ERROR: {receipt_id=\"%s\", content_type=\"%s\", content_length=\"%s\", message=\"%s\"}.\n",
            receipt_id, content_type, content_length, message);
}

static void
_stomp_receipt(char* receipt_id) {
    PRINTA("RECEIPT: {receipt_id=\"%s\"}.\n", receipt_id);
}

static void
_stomp_connected(char* version, char* server, char* host_id, char* session, char* heart_beat, char* user_id) {
    PRINTA("CONNECTED: {version=\"%s\", server=\"%s\", host_id=\"%s\", session=\"%s\", heart_beat=\"%s\", user_id=\"%s\"}.\n",
            version, server, host_id, session, heart_beat, user_id);
}

void
stomp_network_connected() {
    PRINTA("Connected.\n");
    process_post(&stomp_test_process, PROCESS_EVENT_CONTINUE, NULL);
}

static void
_rand_buffer(char *buffer, int size) {
    int i;

    memset(buffer, 0, size);
    for (i = 0; i < size; i++) {
        buffer[i] = rand() % ('z' - 'a') + 'a';
    }
}

int count = 0, i = 0, test = 0;
char buffer[BUFFER_SIZE];

#define NL "\x0a"
#define COLON "\x3a"

PROCESS_THREAD(stomp_test_process, ev, data) {

    PROCESS_BEGIN();

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&ipaddr, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
#else
    uip_ipaddr(&ipaddr, addr[0], addr[1], addr[2], addr[3]);
#endif

    PRINTA("Testing sending function.\n");
    stomp_connect(NULL, NULL, NULL);
    stomp_subscribe(NULL, NULL, NULL);
    stomp_subscribe(NULL, NULL, "");
    stomp_subscribe(NULL, "", "");
    stomp_unsubscribe(NULL);
    stomp_send(NULL, NULL, NULL, NULL, NULL, NULL);
    stomp_send(NULL, NULL, "0", NULL, NULL, NULL);
    stomp_send(NULL, "text/plain", "0", NULL, NULL, NULL);
    stomp_ack(NULL, NULL, NULL);
    stomp_ack(NULL, NULL, "");
    stomp_ack(NULL, "", "");
    stomp_nack(NULL, NULL, NULL);
    stomp_nack(NULL, NULL, "");
    stomp_nack(NULL, "", "");
    stomp_begin(NULL);
    stomp_commit(NULL);
    stomp_abort(NULL);

    PRINTA("Testing receiving function.\n");
    stomp_network_received("MESSAGE" NL
            "destination" COLON "/queue/testing" NL
            "message-id" COLON "message-1" NL
            "subscription" COLON "income" NL
            "content-type" COLON "text/plain" NL
            "content-length" COLON "11" NL
            NL
            "Hello World", 130);
    stomp_network_received("ERROR" NL
            "receipt-id" COLON "receipt-1" NL
            "content-type" COLON "text/plain" NL
            "content-length" COLON "11" NL
            NL
            "Hello World", 81);
    stomp_network_received("RECEIPT" NL
            "receipt-id" COLON "receipt-1" NL
            NL, 30);
    stomp_network_received("CONNECTED" NL
            "version" COLON "1.1" NL
            "server" COLON "apollo" NL
            "session" COLON "session-1" NL
            "host-id" COLON "apollo-1" NL
            "heart-beat" COLON "100,1000" NL
            "user-id" COLON "admin" NL
            NL, 106);

    STOMP_REGISTER_STOMP_SENT(_stomp_sent);
    STOMP_REGISTER_STOMP_RECEIVED(_stomp_received);
    STOMP_REGISTER_STOMP_CONNECTED(_stomp_connected);
    STOMP_REGISTER_STOMP_MESSAGE(_stomp_message_auto);
    STOMP_REGISTER_STOMP_ERROR(_stomp_error);
    STOMP_REGISTER_STOMP_RECEIPT(_stomp_receipt);

    PRINTA("Start. Press any key...\n");
    getchar();

    PRINTA("Waiting for connection...\n");
    STOMP_NETWORK_CONNECT(&ipaddr, port);

    PRINTA("Test: Sending.\n");
    for (test = 0; test < 10; test++) {
        STOMP_CONNECT("apollo", "admin", "password");
        for (count = 1; count < BUFFER_SIZE; count++) {
            _rand_buffer(buffer, count);
            STOMP_SEND("/queue/testing", "text/plain", NULL, NULL, NULL, buffer);
        }
        STOMP_DISCONNECT("1");
    }

    PRINTA("Test: Subscribe & Send.\n");
    for (test = 0; test < 10; test++) {
        STOMP_CONNECT("apollo", "admin", "password");
        STOMP_SUBSCRIBE("income", "/queue/in", NULL);
        for (count = 1; count < BUFFER_SIZE; count++) {
            _rand_buffer(buffer, count);
            STOMP_SEND("/queue/out", "text/plain", NULL, NULL, NULL, buffer);
        }
        STOMP_UNSUBSCRIBE("income");
        STOMP_DISCONNECT("1");
    }

    STOMP_REGISTER_STOMP_MESSAGE(_stomp_message_client_individual);

    PRINTA("Test: Subscribe client-individual & Send.\n");
    for (test = 0; test < 10; test++) {
        STOMP_CONNECT("apollo", "admin", "password");
        STOMP_SUBSCRIBE("income", "/queue/in", "client-individual");
        for (count = 1; count < BUFFER_SIZE; count++) {
            _rand_buffer(buffer, count);
            STOMP_SEND("/queue/out", "text/plain", NULL, NULL, NULL, buffer);
        }
        STOMP_UNSUBSCRIBE("income");
        STOMP_DISCONNECT("1");
    }

    STOMP_REGISTER_STOMP_MESSAGE(_stomp_message_auto);

    PRINTA("Test: Subscribe & Send & Commit.\n");
    for (test = 0; test < 10; test++) {
        STOMP_CONNECT("apollo", "admin", "password");
        STOMP_SUBSCRIBE("income", "/queue/in", NULL);
        STOMP_BEGIN("tx");
        for (count = 1; count < BUFFER_SIZE; count++) {
            _rand_buffer(buffer, count);
            STOMP_SEND("/queue/out", "text/plain", NULL, NULL, "tx", buffer);
        }
        STOMP_COMMIT("tx");
        STOMP_UNSUBSCRIBE("income");
        STOMP_DISCONNECT("1");
    }

    PRINTA("Test: Subscribe & Send & Abort.\n");
    for (test = 0; test < 10; test++) {
        STOMP_CONNECT("apollo", "admin", "password");
        STOMP_SUBSCRIBE("income", "/queue/in", NULL);
        STOMP_BEGIN("tx");
        for (count = 1; count < BUFFER_SIZE; count++) {
            _rand_buffer(buffer, count);
            STOMP_SEND("/queue/out", "text/plain", NULL, NULL, NULL, buffer);
        }
        STOMP_ABORT("tx");
        STOMP_UNSUBSCRIBE("income");
        STOMP_DISCONNECT("1");
    }

    PROCESS_END();
}