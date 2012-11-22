#include "stomp-sensor.h"

#include <stdio.h>
#include <stdlib.h>

#define STOMP_SENSORS_TICK 40

PROCESS(stomp_sensors, "STOMP sensors process");

PROCESS_THREAD(stomp_sensors, ev, data) {
    static struct etimer et;
    static int8_t tick;
    static struct stomp_sensor **sens;

    PROCESS_BEGIN();
    tick = 0;
    etimer_set(&et, CLOCK_CONF_SECOND * 15);

    while (1) {
        PROCESS_WAIT_EVENT();
        /*
         * 15s 1 1 1 1  1 1 1 1  1 1 1 1  1 1 1 1  1 1 1 1  1 1 1 1  1 1 1 1  1 1 1 1  1 1 1 1  1 1 1 1 - co 1  tick
         * 30s 1 0 1 0  1 0 1 0  1 0 1 0  1 0 1 0  1 0 1 0  1 0 1 0  1 0 1 0  1 0 1 0  1 0 1 0  1 0 1 0 - co 2  tick'i
         * 1m  1 0 0 0  1 0 0 0  1 0 0 0  1 0 0 0  1 0 0 0  1 0 0 0  1 0 0 0  1 0 0 0  1 0 0 0  1 0 0 0 - co 4  tick'i
         * 2m  1 0 0 0  0 0 0 0  1 0 0 0  0 0 0 0  1 0 0 0  0 0 0 0  1 0 0 0  0 0 0 0  1 0 0 0  0 0 0 0 - co 8  tick'ow
         * 5m  1 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  1 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0 - co 20 tick'ow
         * 10m 1 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0 - co 40 tick'ow
         */
        tick = (tick + 1) % STOMP_SENSORS_TICK;
        sens = stomp_sensor_processes;
        while (sens != NULL && *sens != NULL) {
            if ((*sens)->update & STOMP_SENSOR_UPDATE_PERIODICALLY) {
                /* PERIODIC = 15s */
                if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_15s) {
                    printf("stomp_sensor: periodic=15s, value=%d.\n", (*sens)->value);
                }
                if (tick % 2 == 0) {
                    /* PERIODIC = 30s */
                    if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_30s) {
                        printf("stomp_sensor: periodic=30s, value=%d.\n", (*sens)->value);
                    }
                    if (tick % 4 == 0) {
                        /* PERIODIC = 1m */
                        if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_1m) {
                            printf("stomp_sensor: periodic=1m, value=%d.\n", (*sens)->value);
                        }
                        if (tick == 0 || tick == 8 || tick == 16 || tick == 24 || tick == 32) {
                            /* PERIODIC = 2m */
                            if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_2m) {
                                printf("stomp_sensor: periodic=2m, value=%d.\n", (*sens)->value);
                            }
                            if (tick == 0 || tick == 20) {
                                /* PERIODIC = 5m */
                                if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_5m) {
                                    printf("stomp_sensor: periodic=5m, value=%d.\n", (*sens)->value);
                                }
                                if (tick == 0) {
                                    /* PERIODIC = 10m */
                                    if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_10m) {
                                        printf("stomp_sensor: periodic=10m, value=%d.\n", (*sens)->value);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if ((*sens)->update & STOMP_SENSOR_UPDATE_ON_CHANGE) {
                if ((*sens)->last != (*sens)->value) {
                    printf("stomp_sensor: changed, value=%d.\n", (*sens)->value);
                }
            }
            sens++;
        }
        etimer_restart(&et);
    }
    PROCESS_END();
}

int16_t stomp_sensor_const(int16_t min, int16_t max, int16_t delta, int16_t step, int16_t value) {
    return value;
}

int16_t stomp_sensor_random(int16_t min, int16_t max, int16_t delta, int16_t step, int16_t value) {
    if (min < max) {
        return (rand() % ((max - min) / step)) * step + min;
    } else {
        return (rand() % ((min - max) / step)) * step + max;
    }
}

int16_t stomp_sensor_random_delta(int16_t min, int16_t max, int16_t delta, int16_t step, int16_t value) {
    if ((rand() % 50) > 25) {
        value = value + (rand() % (delta / step)) * step;
    } else {
        value = value - (rand() % (delta / step)) * step;
    }
    if (min < max) {
        if (value < min) {
            value = min;
        } else if (value > max) {
            value = max;
        }
    } else {
        if (value < max) {
            value = max;
        } else {
            value = min;
        }
    }
    return value;
}