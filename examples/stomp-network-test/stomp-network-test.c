#include <string.h>

#include "stomp-strings.h"
#include "stomp-tools.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"
#include "stomp-network.h"

#if CONTIKI_TARGET_MINIMAL_NET > 0
#include <stdlib.h>
#include <sys/time.h>
#elif CONTIKI_TARGET_AVR_ZIGDUINO > 0
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#endif

#define BUFFER_SIZE 1200
#define STOMP_TEST 20

#if UIP_CONF_IPV6 > 0
int addr[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
#else
int addr[] = {10, 1, 1, 100};
#endif

uip_ipaddr_t ipaddr;
int port = 61613;

int count = 0, i = 0, test = 0;
char *buffer;

PROCESS(stomp_network_test_process, "STOMP network test");
AUTOSTART_PROCESSES(&stomp_network_test_process, &stomp_network_process);

#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
uint8_t sreg;
uint16_t __t1, __t2, __t3, __t4, __count1, __count2, __count3;
unsigned long int __sum1, __sum2, __sum3;

ISR(TIMER4_OVF_vect) {
    printf("Overflow!\n");
}
#endif

void
stomp_network_connected() {
#if STOMP_DEBUG > 1
    PRINTA("Connected.\n");
#endif
    process_post(&stomp_network_test_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_network_sent(char *buf, int len) {
#if STOMP_DEBUG > 1
    PRINTA("Sent:     {buf=\"%s\", len=\"%d\"}.\n", buf, len);
#endif
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
    sreg = SREG;
    cli();
    __t2 = TCNT4;
    SREG = sreg;
    if (__t2 > __t1) {
        __sum1 += (__t2 - __t1);
        __count1 += 1;
    }
#endif
    process_post(&stomp_network_test_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_network_received(char *buf, int len) {
#if STOMP_DEBUG > 1
    PRINTA("Received: {buf=\"%s\", len=\"%d\"}.\n", buf, len);
#endif
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
    sreg = SREG;
    cli();
    __t4 = TCNT4;
    SREG = sreg;
    if (__t4 > __t1) {
        __sum3 += (__t4 - __t1);
        __count3 += 1;
    }
#endif
}

static void
_rand_buffer(char *buffer, int size) {
    int i;

    memset(buffer, 0, size);
    for (i = 0; i < size; i++) {
        buffer[i] = rand() % ('z' - 'a') + 'a';
    }
}

PROCESS_THREAD(stomp_network_test_process, ev, data) {

    static struct etimer et;

    PROCESS_BEGIN();

#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
    sreg = SREG;
    cli();
    TCCR4B = 0;
    // TIMSK4 = (1 << TOIE4);
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

#if CONTIKI_TARGET_MINIMAL_NET > 0
    PRINTA("Start. Press any key...\n");
    getchar();
#endif

    PRINTA("Waiting for connection...\n");
    STOMP_NETWORK_CONNECT(&ipaddr, port);

    PRINTA("Test: Sending & Receiving.\n");
    for (count = 10; count < BUFFER_SIZE; count += 10) {
        printf("BUFFER_SIZE: %d\n", count);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
        __sum1 = 0;
        __sum2 = 0;
        __sum3 = 0;
        __count1 = 0;
        __count2 = 0;
        __count3 = 0;
#endif
        for (test = 0; test < STOMP_TEST; test++) {
            buffer = NEW_ARRAY(char, count);
            _rand_buffer(buffer, count);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            __t1 = TCNT4;
            SREG = sreg;
#endif
            stomp_network_send(buffer, count);
            etimer_set(&et, 250);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE || ev == PROCESS_EVENT_TIMER);
            if (ev != PROCESS_EVENT_TIMER) etimer_stop(&et);
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
            sreg = SREG;
            cli();
            __t3 = TCNT4;
            SREG = sreg;
            if (__t3 > __t2) {
                __sum2 += (__t3 - __t2);
                __count2 += 1;
            }
#endif
            DELETE(buffer);
        }
#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
        printf("%lu %lu %lu %u %u %u\n",
                __sum1, __sum2, __sum3, __count1, __count2, __count3);
#endif
    }

    PROCESS_END();
}
