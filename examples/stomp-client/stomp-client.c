#include "stomp.h"
#include "stomp-sensor.h"
#include "stomp-network.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

STOMP_SENSOR(stomp_sensor_temperature, 2, stomp_sensor_random_delta, 15, -30, 40, 10, 1);
STOMP_SENSOR(stomp_sensor_humidity, 2, stomp_sensor_random_delta, 60, 0, 100, 20, 5);
STOMP_SENSOR(stomp_sensor_pressure, 2, stomp_sensor_random_delta, 1000, 800, 1200, 50, 5);

STOMP_SENSOR_PROCESSES(&stomp_sensor_temperature_data, &stomp_sensor_humidity_data, &stomp_sensor_pressure_data);

PROCESS(stomp_client_process, "STOMP contiki client");
AUTOSTART_PROCESSES(&stomp_client_process, &stomp_network_process, &stomp_network_send_process, &stomp_sensors, &stomp_sensor_temperature, &stomp_sensor_humidity, &stomp_sensor_pressure);

PROCESS_THREAD(stomp_client_process, ev, data) {
    PROCESS_BEGIN();

    srand(time(NULL));

    stomp_sensor_temperature_data.periodic = STOMP_SENSOR_PERIODIC_15s;
    stomp_sensor_humidity_data.periodic = STOMP_SENSOR_PERIODIC_30s;
    stomp_sensor_pressure_data.periodic = STOMP_SENSOR_PERIODIC_1m;

    stomp_sensor_temperature_data.update = STOMP_SENSOR_UPDATE_PERIODICALLY;
    stomp_sensor_humidity_data.update = STOMP_SENSOR_UPDATE_ON_CHANGE;
    stomp_sensor_pressure_data.update = STOMP_SENSOR_UPDATE_PERIODICALLY_ON_CHANGE;

    printf("Press any key...\n");
    getchar();

    printf("Waiting for server...\n");

    stomp_connect("apollo", "admin", "password");

    stomp_send("/queue/a", "text/plain", NULL, NULL, NULL, "Testowa wiadomosc, wysylana na serwer");

    stomp_disconnect("0");

    PROCESS_END();
}

void
stomp_connected() {
    printf("Hooray! We have been connected to server!\n");
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_sent() {
    printf("Yupie! It has been sent!\n");
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_received(struct stomp_frame *frame) {
    printf("Yeah! We have got it!\n");
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}

void
stomp_closed() {
    printf("Oh nooo! The connection has been closed!\n");
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}