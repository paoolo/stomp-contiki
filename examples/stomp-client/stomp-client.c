#include "ultra-simple-stomp.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "uip-debug.h" 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
#include "stomp.h"
#include "stomp-sensor.h"
#include "stomp-network.h"

#define STOMP_SENSOR_UUID "203284818775978"

STOMP_SENSOR(stomp_sensor_temperature, 2, stomp_sensor_random_delta, 15, -30, 40, 10, 1);
STOMP_SENSOR(stomp_sensor_humidity, 2, stomp_sensor_random_delta, 60, 0, 100, 20, 5);
STOMP_SENSOR(stomp_sensor_pressure, 2, stomp_sensor_random_delta, 1000, 800, 1200, 50, 5);

STOMP_SENSOR_PROCESSES(&stomp_sensor_temperature_data, &stomp_sensor_humidity_data, &stomp_sensor_pressure_data);
 */

PROCESS(stomp_client_process, "STOMP contiki client");
AUTOSTART_PROCESSES(&stomp_client_process /*, &stomp_network_process, &stomp_network_send_process, &stomp_sensors, &stomp_sensor_temperature, &stomp_sensor_humidity, &stomp_sensor_pressure */);

PROCESS_THREAD(stomp_client_process, ev, data) {
//    static struct etimer et;
    PROCESS_BEGIN();

    PRINTA("Start.\n");
    getchar();
    /*
        stomp_sensor_temperature_data.periodic = STOMP_SENSOR_PERIODIC_15s;
        stomp_sensor_humidity_data.periodic = STOMP_SENSOR_PERIODIC_30s;
        stomp_sensor_pressure_data.periodic = STOMP_SENSOR_PERIODIC_1m;

        stomp_sensor_temperature_data.update = STOMP_SENSOR_UPDATE_PERIODICALLY;
        stomp_sensor_humidity_data.update = STOMP_SENSOR_UPDATE_ON_CHANGE;
        stomp_sensor_pressure_data.update = STOMP_SENSOR_UPDATE_PERIODICALLY_ON_CHANGE;
     */

    stomp_connect("apollo", "admin", "password");
    stomp_subscribe("id-203284818775978", "/queue/203284818775978", "auto");
    stomp_begin("tx");
    stomp_send("/queue/all", "text/plain", NULL, NULL, NULL, "HELLO");
    stomp_abort("tx");
    stomp_begin("tx");
    stomp_send("/queue/all", "text/plain", NULL, NULL, NULL, "HELLO");
    stomp_commit("tx");
    stomp_unsubscribe("id-203284818775978");
    stomp_disconnect("0");
    
//    etimer_set(&et, CLOCK_CONF_SECOND * 2);
//    while (1) {
//        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
//        stomp_send("/queue/all", "text/plain", NULL, NULL, NULL, "HELLO");
//        etimer_set(&et, CLOCK_CONF_SECOND * 2);
//    }
    

    PRINTA("Stop.\n");

    PROCESS_END();
}