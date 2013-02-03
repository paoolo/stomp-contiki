#include <string.h>

#include "stomp.h"
#include "stomp-strings.h"
#include "stomp-tools.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"
#include "stomp-network.h"

#if CONTIKI_TARGET_MINIMAL_NET > 0
#include <stdlib.h>
#include <sys/time.h>
#endif

#define BUFFER_SIZE 128
#define STOMP_TEST 5

#if UIP_CONF_IPV6 > 0
int addr[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
// int addr[] = {0x2001, 0x470, 0x1f0a, 0xbf2, 0, 0, 0, 0x2};
#else
int addr[] = {10, 1, 1, 100};
#endif

uip_ipaddr_t ipaddr;
int port = 61613;

PROCESS(stomp_test_process, "STOMP test");
AUTOSTART_PROCESSES(&stomp_test_process, &stomp_network_process);

#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
uint8_t sreg;

ISR(TIMER4_OVF_vect) {
    printf("Overflow!\n");
}
#endif

static void
_stomp_sent(char *buf, int len) {
#if STOMP_DEBUG > 1
    PRINTA("Sent frame: {buf=\"%s\", len=\"%d\"}\n", buf, len);
#endif
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
    sreg = SREG;
    cli();
    PRINTA("2;%" PRIu16 ";-1\n", TCNT4);
    SREG = sreg;
#endif
    process_post(&stomp_test_process, PROCESS_EVENT_CONTINUE, NULL);
}

static void
_stomp_received(char *buf, int len) {
#if STOMP_DEBUG > 1
    PRINTA("Received frame: {buf=\"%s\", len=\"%d\"}\n", buf, len);
#endif
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
    sreg = SREG;
    cli();
    PRINTA("4;%" PRIu16 ";-1\n", TCNT4);
    SREG = sreg;
#endif
}

static void
_stomp_message_auto(char* destination, char* message_id, char* subscription, char* content_type, char* content_length, char* message) {
#if STOMP_DEBUG > 1
    PRINTA("MESSAGE: {destination=\"%s\", message_id=\"%s\", subscription=\"%s\", content_type=\"%s\", content_length=\"%s\", message=\"%s\"}.\n",
            destination, message_id, subscription, content_type, content_length, message);
#endif
}

static void
_stomp_message_client_individual(char* destination, char* message_id, char* subscription, char* content_type, char* content_length, char* message) {
#if STOMP_DEBUG > 1    
    PRINTA("MESSAGE: {destination=\"%s\", message_id=\"%s\", subscription=\"%s\", content_type=\"%s\", content_length=\"%s\", message=\"%s\"}.\n",
            destination, message_id, subscription, content_type, content_length, message);
#endif
    stomp_ack(subscription, message_id, NULL);
}

static void
_stomp_error(char* receipt_id, char* content_type, char* content_length, char *message) {
#if STOMP_DEBUG > 1
    PRINTA("ERROR: {receipt_id=\"%s\", content_type=\"%s\", content_length=\"%s\", message=\"%s\"}.\n",
            receipt_id, content_type, content_length, message);
#endif
}

static void
_stomp_receipt(char* receipt_id) {
#if STOMP_DEBUG > 1
    PRINTA("RECEIPT: {receipt_id=\"%s\"}.\n", receipt_id);
#endif
}

static void
_stomp_connected(char* version, char* server, char* host_id, char* session, char* heart_beat, char* user_id) {
#if STOMP_DEBUG > 1
    PRINTA("CONNECTED: {version=\"%s\", server=\"%s\", host_id=\"%s\", session=\"%s\", heart_beat=\"%s\", user_id=\"%s\"}.\n",
            version, server, host_id, session, heart_beat, user_id);
#endif
}

void
stomp_network_connected() {
#if STOMP_DEBUG > 1
    PRINTA("Connected.\n");
#endif
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

    static struct etimer et;

    PROCESS_BEGIN();

#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
    sreg = SREG;
    cli();
    TCCR4B = 0;
    TIMSK4 = (1 << TOIE4);
    TCCR4B |= (1 << CS10);
    TCCR4B |= (1 << CS12);
    TCNT4 = 0;
    SREG = sreg;
#endif

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

#if CONTIKI_TARGET_MINIMAL_NET > 0
    PRINTA("Start. Press any key...\n");
    getchar();
#endif

    PRINTA("Waiting for connection...\n");
    STOMP_NETWORK_CONNECT(&ipaddr, port);

    PRINTA("Test: Sending.\n");
    STOMP_CONNECT("apollo", "admin", "password");
    for (count = 1; count < BUFFER_SIZE; count++) {
        for (test = 0; test < STOMP_TEST; test++) {
            _rand_buffer(buffer, count);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("1;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
            stomp_send("/queue/testing", "text/plain", NULL, NULL, NULL, buffer);
            etimer_set(&et, 250);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE || ev == PROCESS_EVENT_TIMER);
            if (ev != PROCESS_EVENT_TIMER) etimer_stop(&et);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("3;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
        }
    }

    PRINTA("Test: Subscribe & Send.\n");
    STOMP_SUBSCRIBE("income", "/queue/in", NULL);
    for (count = 1; count < BUFFER_SIZE; count++) {
        for (test = 0; test < STOMP_TEST; test++) {
            _rand_buffer(buffer, count);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("1;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
            stomp_send("/queue/out", "text/plain", NULL, NULL, NULL, buffer);
            etimer_set(&et, 250);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE || ev == PROCESS_EVENT_TIMER);
            if (ev != PROCESS_EVENT_TIMER) etimer_stop(&et);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("3;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
        }
    }
    STOMP_UNSUBSCRIBE("income");

    PRINTA("Test: Subscribe client-individual & Send.\n");
    STOMP_REGISTER_STOMP_MESSAGE(_stomp_message_client_individual);
    STOMP_SUBSCRIBE("income", "/queue/in", "client-individual");
    for (count = 1; count < BUFFER_SIZE; count++) {
        for (test = 0; test < STOMP_TEST; test++) {
            _rand_buffer(buffer, count);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("1;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
            stomp_send("/queue/out", "text/plain", NULL, NULL, NULL, buffer);
            etimer_set(&et, 250);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE || ev == PROCESS_EVENT_TIMER);
            if (ev != PROCESS_EVENT_TIMER) etimer_stop(&et);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("3;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
        }
    }
    STOMP_UNSUBSCRIBE("income");

    PRINTA("Test: Subscribe & Send & Commit.\n");
    STOMP_REGISTER_STOMP_MESSAGE(_stomp_message_auto);
    STOMP_SUBSCRIBE("income", "/queue/in", NULL);
    STOMP_BEGIN("tx");
    for (count = 1; count < BUFFER_SIZE; count++) {
        for (test = 0; test < STOMP_TEST; test++) {
            _rand_buffer(buffer, count);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("1;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
            stomp_send("/queue/out", "text/plain", NULL, NULL, "tx", buffer);
            etimer_set(&et, 250);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE || ev == PROCESS_EVENT_TIMER);
            if (ev != PROCESS_EVENT_TIMER) etimer_stop(&et);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("3;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
        }
    }
    STOMP_COMMIT("tx");
    STOMP_UNSUBSCRIBE("income");

    PRINTA("Test: Subscribe & Send & Abort.\n");
    STOMP_SUBSCRIBE("income", "/queue/in", NULL);
    STOMP_BEGIN("tx");
    for (count = 1; count < BUFFER_SIZE; count++) {
        for (test = 0; test < STOMP_TEST; test++) {
            _rand_buffer(buffer, count);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("1;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
            stomp_send("/queue/out", "text/plain", NULL, NULL, NULL, buffer);
            etimer_set(&et, 250);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE || ev == PROCESS_EVENT_TIMER);
            if (ev != PROCESS_EVENT_TIMER) etimer_stop(&et);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            PRINTA("3;%" PRIu16 ";%d\n", TCNT4, count);
            SREG = sreg;
#endif
        }
    }
    STOMP_ABORT("tx");
    STOMP_UNSUBSCRIBE("income");
    STOMP_DISCONNECT("1");

    PROCESS_END();
}
