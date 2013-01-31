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

#if CONTIKI_TARGET_MINIMAL_NET > 0
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
uint16_t __profile_tm, __profile_tm_start, __profile_tm_stop;

static void
__profile_print_tm() {
    uint8_t sreg;

    __profile_tm = clock_seconds();
    /*
    cli();
    sreg = SREG;
    __profile_tm = TCNT1;
    SREG = sreg;
    sei();
     */

#if STOMP_PROFILE > 1
    printf("%d\n", __profile_tm);
#endif
}

static void
__profile_start() {
    uint8_t sreg;

    __profile_tm_start = clock_seconds();

    /*
    cli();
    sreg = SREG;
    __profile_tm_start = TCNT1;
    SREG = sreg;
    sei();
     */
}

static void
__profile_stop() {
    uint8_t sreg;

    __profile_tm_stop = clock_seconds();
    /*
    cli();
    sreg = SREG;
    __profile_tm_stop = TCNT1;
    SREG = sreg;
    sei();
     */

    __profile_sum += (__profile_tm_stop - __profile_tm_start);
    __profile_count += 1;
#if STOMP_PROFILE > 1
    printf("%ld\n", __profile_tm_stop - __profile_tm_start);
#endif
}
#endif
#endif

PROCESS_THREAD(stomp_network_test_process, ev, data) {

    PROCESS_BEGIN();

#if STOMP_PROFILE > 0 && CONTIKI_TARGET_AVR_ZIGDUINO > 0
    __profile_tm = 0;
    __profile_tm_start = 0;
    __profile_tm_stop = 0;

    __profile_print_tm();
#endif

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&ipaddr, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
#else
    uip_ipaddr(&ipaddr, addr[0], addr[1], addr[2], addr[3]);
#endif

    PRINTA("Start. Press any key...\n");
    getchar();

#if STOMP_DEBUG > 1
    PRINTA("Waiting for connection...\n");
#endif
    STOMP_NETWORK_CONNECT(&ipaddr, port);

#if STOMP_DEBUG > 0
    PRINTA("Test: Sending & Receiving.\n");
#endif

    for (count = 1; count < BUFFER_SIZE; count++) {
#if STOMP_PROFILE > 0
        printf("BUFFER_SIZE: %d\n", count);
        __profile_reset();
#endif
        for (test = 0; test < STOMP_TEST; test++) {
            buffer = NEW_ARRAY(char, count);
            _rand_buffer(buffer, count);
#if STOMP_PROFILE > 0
            __profile_start();
#endif
            stomp_network_send(buffer, count);
            PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
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