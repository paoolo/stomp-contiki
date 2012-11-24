#include "ultra-simple-stomp.h"
#include "stomp-sensor.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"
#include "ultra-simple-stomp-network.h"

#define STOMP_SENSOR_UUID "sensor1"
#define STOMP_SENSORS_TICK 40

STOMP_SENSOR(stomp_sensor_temperature, 2, stomp_sensor_random_delta, 15, -30, 40, 10, 1);
STOMP_SENSOR(stomp_sensor_humidity, 2, stomp_sensor_random_delta, 60, 0, 100, 20, 5);
STOMP_SENSOR(stomp_sensor_pressure, 2, stomp_sensor_random_delta, 1000, 800, 1200, 50, 5);

STOMP_SENSOR_PROCESSES(&stomp_sensor_temperature_data, &stomp_sensor_humidity_data, &stomp_sensor_pressure_data);

PROCESS(stomp_client_process, "STOMP client");
AUTOSTART_PROCESSES(&stomp_client_process, &ultra_simple_stomp_network_process, &stomp_sensor_temperature, &stomp_sensor_humidity, &stomp_sensor_pressure);

PROCESS_THREAD(stomp_client_process, ev, data) {
    static struct etimer et;
    static int tick;
    static struct stomp_sensor **sens;
    static char tmp1[64], tmp2[64];

    PROCESS_BEGIN();

    PRINTA("Start. Press any key...\n");
    getchar();

    PRINTA("Waiting for connection...\n");
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

    STOMP_CONNECT("apollo", "admin", "password");
    STOMP_SUBSCRIBE("income", "/queue/" STOMP_SENSOR_UUID, "auto");
    STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, "HELLO;" STOMP_SENSOR_UUID);

    sens = stomp_sensor_processes;
    while (sens != NULL && *sens != NULL) {
        sprintf(tmp1, "income%s", (*sens)->name);
        sprintf(tmp2, "/queue/%s", (*sens)->name);
        STOMP_SUBSCRIBE(tmp1, tmp2, "auto");
        sens++;
    }

    stomp_sensor_temperature_data.periodic = STOMP_SENSOR_PERIODIC_15s;
    stomp_sensor_humidity_data.periodic = STOMP_SENSOR_PERIODIC_30s;
    stomp_sensor_pressure_data.periodic = STOMP_SENSOR_PERIODIC_1m;

    stomp_sensor_temperature_data.update = STOMP_SENSOR_UPDATE_PERIODICALLY;
    stomp_sensor_humidity_data.update = STOMP_SENSOR_UPDATE_ON_CHANGE;
    stomp_sensor_pressure_data.update = STOMP_SENSOR_UPDATE_PERIODICALLY_ON_CHANGE;

    tick = 0;
    etimer_set(&et, CLOCK_CONF_SECOND * 15);

    while (1) {
        PROCESS_WAIT_EVENT();
        tick = (tick + 1) % STOMP_SENSORS_TICK;
        sens = stomp_sensor_processes;
        STOMP_BEGIN("tx");
        while (sens != NULL && *sens != NULL) {
            if ((*sens)->update & STOMP_SENSOR_UPDATE_PERIODICALLY) {

                if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_15s) {
                    sprintf(tmp1, "UDPATE;" STOMP_SENSOR_UUID ";%s;%d", (*sens)->name, (*sens)->value);
                    STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                }
                if (tick % 2 == 0) {
                    if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_30s) {
                        sprintf(tmp1, "UDPATE;" STOMP_SENSOR_UUID ";%s;%d", (*sens)->name, (*sens)->value);
                        STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                    }
                    if (tick % 4 == 0) {
                        if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_1m) {
                            sprintf(tmp1, "UDPATE;" STOMP_SENSOR_UUID ";%s;%d", (*sens)->name, (*sens)->value);
                            STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                        }
                        if (tick == 0 || tick == 8 || tick == 16 || tick == 24 || tick == 32) {
                            if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_2m) {
                                sprintf(tmp1, "UDPATE;" STOMP_SENSOR_UUID ";%s;%d", (*sens)->name, (*sens)->value);
                                STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                            }
                            if (tick == 0 || tick == 20) {
                                if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_5m) {
                                    sprintf(tmp1, "UDPATE;" STOMP_SENSOR_UUID ";%s;%d", (*sens)->name, (*sens)->value);
                                    STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                                }
                                if (tick == 0) {
                                    if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_10m) {
                                        sprintf(tmp1, "UDPATE;" STOMP_SENSOR_UUID ";%s;%d", (*sens)->name, (*sens)->value);
                                        STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if ((*sens)->update & STOMP_SENSOR_UPDATE_ON_CHANGE) {
                if ((*sens)->last != (*sens)->value) {
                    sprintf(tmp1, "UDPATE;" STOMP_SENSOR_UUID ";%s;%d", (*sens)->name, (*sens)->value);
                    STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                }
            }
            sens++;
        }
        STOMP_COMMIT("tx");
        etimer_restart(&et);
    }
    PROCESS_END();
}

void
stomp_sent() {
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_received(char *buf, int len) {
    /* TODO parsowanie ramek */
}

void
stomp_connected() {
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}