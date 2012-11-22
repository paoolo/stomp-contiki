#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#ifndef STOMP_SENSOR_H
#define	STOMP_SENSOR_H

#define STOMP_SENSOR_NO_UPDATE                          0b00
#define STOMP_SENSOR_UPDATE_PERIODICALLY                0b01
#define STOMP_SENSOR_UPDATE_ON_CHANGE                   0b10
#define STOMP_SENSOR_UPDATE_PERIODICALLY_ON_CHANGE      0b11

#define STOMP_SENSOR_PERIODIC_15s       0b100000
#define STOMP_SENSOR_PERIODIC_30s       0b010000
#define STOMP_SENSOR_PERIODIC_1m        0b001000
#define STOMP_SENSOR_PERIODIC_2m        0b000100
#define STOMP_SENSOR_PERIODIC_5m        0b000010
#define STOMP_SENSOR_PERIODIC_10m       0b000001
#define STOMP_SENSOR_PERIODIC_NONE      0b000000

struct stomp_sensor {
    int8_t update;
    int8_t periodic;
    int16_t last;
    int16_t value;
};

#define STOMP_SENSOR(PROCESS_NAME, FREQ, NEXT_VALUE, INIT, MIN, MAX, DELTA, STEP) \
static struct stomp_sensor PROCESS_NAME##_data; \
PROCESS(PROCESS_NAME, "STOMP sensor " #PROCESS_NAME); \
PROCESS_THREAD(PROCESS_NAME, ev, data) { \
    static struct etimer et; \
    PROCESS_BEGIN(); \
    PROCESS_NAME##_data.value = INIT; \
    etimer_set(&et, CLOCK_CONF_SECOND * FREQ); \
    while (1) { \
        PROCESS_WAIT_EVENT(); \
        PROCESS_NAME##_data.last = PROCESS_NAME##_data.value; \
        PROCESS_NAME##_data.value = NEXT_VALUE(MIN, MAX, DELTA, STEP, PROCESS_NAME##_data.value); \
        printf(#PROCESS_NAME "_value = %d\n", PROCESS_NAME##_data.value); \
        etimer_restart(&et); \
    } \
    PROCESS_END(); \
}

extern struct stomp_sensor * const stomp_sensor_processes[];

#if ! CC_NO_VA_ARGS
#define STOMP_SENSOR_PROCESSES(...) \
struct stomp_sensor * const stomp_sensor_processes[] = {__VA_ARGS__, NULL}
#else
#error "C compiler must support __VA_ARGS__ macro"
#endif

PROCESS_NAME(stomp_sensors);

int16_t stomp_sensor_const(int16_t min, int16_t max, int16_t delta, int16_t step, int16_t value);

int16_t stomp_sensor_random(int16_t min, int16_t max, int16_t delta, int16_t step, int16_t value);

int16_t stomp_sensor_random_delta(int16_t min, int16_t max, int16_t delta, int16_t step, int16_t value);

#endif	/* STOMP_SENSOR_H */

