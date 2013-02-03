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

#define BUFFER_SIZE 256
#define STOMP_TEST 10

#if UIP_CONF_IPV6 > 0
int addr[] = {0xfe80, 0, 0, 0, 0, 0, 0, 1};
// int addr[] = {0x2001, 0x470, 0x1f0a, 0xbf2, 0, 0, 0, 0x2};
#else
int addr[] = {10, 1, 1, 100};
#endif

uip_ipaddr_t ipaddr;
int port = 61613;

int count = 0, i = 0, test = 0;
char *buffer;

PROCESS(stomp_network_test_process, "STOMP network test");
AUTOSTART_PROCESSES(&stomp_network_test_process, &stomp_network_process);

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
    process_post(&stomp_network_test_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_network_received(char *buf, int len) {
#if STOMP_DEBUG > 1
    PRINTA("Received: {buf=\"%s\", len=\"%d\"}.\n", buf, len);
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

#if STOMP_PROFILE > 0
#if CONTIKI_TARGET_MINIMAL_NET > 0
long int __profile_sum, __profile_count;

static void
__profile_reset() {
    __profile_sum = 0;
    __profile_count = 0;
}

static void
__profile_avg() {
    printf("%g\n", (double) __profile_sum / (double) __profile_count);
}

struct timeval __profile_tv, __profile_tv_start, __profile_tv_stop;
struct timezone __profile_tz, __profile_tz_start, __profile_tz_stop;

static void
__profile_print_tm() {
    gettimeofday(&__profile_tv, &__profile_tz);
#if STOMP_PROFILE > 1
    printf("%ld %ld\n", __profile_tv.tv_sec, __profile_tv.tv_usec);
#endif
}

static void
__profile_start() {
    gettimeofday(&__profile_tv_start, &__profile_tz_start);
}

static void
__profile_stop() {
    gettimeofday(&__profile_tv_stop, &__profile_tz_stop);
    __profile_sum += (__profile_tv_stop.tv_usec - __profile_tv_start.tv_usec)
            + (__profile_tv_stop.tv_sec - __profile_tv_start.tv_sec) * 1000000;
    __profile_count += 1;
#if STOMP_PROFILE > 1
    printf("%ld %ld\n",
            __profile_tv_stop.tv_sec - __profile_tv_start.tv_sec,
            __profile_tv_stop.tv_usec - __profile_tv_start.tv_usec);
#endif
}

#elif CONTIKI_TARGET_AVR_ZIGDUINO > 0
uint16_t __profile_sum, __profile_count;

#define __profile_reset() \
    __profile_sum = 0; \
    __profile_count = 0;

#define __profile_avg() \
    printf("__profile_avg() -> %" PRIu16 " %" PRIu16 "\n", __profile_sum, __profile_count);

uint8_t sreg;
uint16_t __profile_tm, __profile_tm_start, __profile_tm_stop;

#define __profile_print_tm() \
    __profile_tm = TCNT4; \
    printf("__profile_print_tm() -> %" PRIu16 "\n", __profile_tm);

#define __profile_start() \
    __profile_tm_start = TCNT4;

#define __profile_stop() \
    __profile_tm_stop = TCNT4; \
    __profile_sum += (__profile_tm_stop - __profile_tm_start); \
    __profile_count += 1;

ISR(TIMER4_OVF_vect) {
    printf("Overflow!\n");
}

#endif
#endif

PROCESS_THREAD(stomp_network_test_process, ev, data) {

    static struct etimer et;

    PROCESS_BEGIN();

#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
    TCCR4B = 0;
    TIMSK4 = (1 << TOIE4);
    TCCR4B |= (1 << CS10);
    TCCR4B |= (1 << CS12);
    TCNT4 = 0;

    __profile_tm = 0;
    __profile_tm_start = 0;
    __profile_tm_stop = 0;
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

#if STOMP_DEBUG > 1
    PRINTA("Waiting for connection...\n");
#endif
    STOMP_NETWORK_CONNECT(&ipaddr, port);
#if STOMP_DEBUG > 0
    PRINTA("Test: Sending & Receiving.\n");
#endif
    for (count = 1; count < BUFFER_SIZE; count++) {
#if STOMP_DEBUG > 0
        printf("BUFFER_SIZE: %d\n", count);
#endif
#if STOMP_PROFILE > 0
        __profile_reset();
#endif
        for (test = 0; test < STOMP_TEST; test++) {
            buffer = NEW_ARRAY(char, count);
            _rand_buffer(buffer, count);
#if STOMP_PROFILE > 0
            __profile_start();
#endif
            stomp_network_send(buffer, count);
            etimer_set(&et, 250);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE || ev == PROCESS_EVENT_TIMER);
            if (ev != PROCESS_EVENT_TIMER) etimer_stop(&et);
#if STOMP_PROFILE > 0
            __profile_stop();
#endif
            DELETE(buffer);
        }
#if STOMP_PROFILE > 0
        __profile_avg();
#endif
    }

    PROCESS_END();
}
