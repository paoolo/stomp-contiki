#include <string.h>

#include "ultra-simple-stomp.h"
#include "stomp-sensor.h"
#include "stomp-tools.h"

#include "contiki.h"
#include "contiki-net.h"

#include "uip-debug.h"
#include "ultra-simple-stomp-network.h"

#define UUID "sensor1"
#define TICKS 40

STOMP_SENSOR(temp, 2, stomp_sensor_random_delta, 15, -30, 40, 10, 1);
STOMP_SENSOR(hum, 2, stomp_sensor_random_delta, 60, 0, 100, 20, 5);
STOMP_SENSOR(pres, 2, stomp_sensor_random_delta, 1000, 800, 1200, 50, 5);

STOMP_SENSOR_PROCESSES(&temp_data, &hum_data, &pres_data);

PROCESS(stomp_client_process, "STOMP client");
AUTOSTART_PROCESSES(&stomp_client_process, &ultra_simple_stomp_network_process, &temp, &hum, &pres);

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
    STOMP_SUBSCRIBE("income", "/queue/" UUID, "auto");
    STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, "HELLO;" UUID "temp,hum,pres,");

    sens = stomp_sensor_processes;
    while (sens != NULL && *sens != NULL) {
        sprintf(tmp1, "income%s", (*sens)->name);
        sprintf(tmp2, "/queue/%s", (*sens)->name);
        STOMP_SUBSCRIBE(tmp1, tmp2, "auto");
        sens++;
    }

    temp_data.periodic = STOMP_SENSOR_PERIODIC_15s;
    hum_data.periodic = STOMP_SENSOR_PERIODIC_30s;
    pres_data.periodic = STOMP_SENSOR_PERIODIC_1m;

    temp_data.update = STOMP_SENSOR_UPDATE_PERIODICALLY;
    hum_data.update = STOMP_SENSOR_UPDATE_ON_CHANGE;
    pres_data.update = STOMP_SENSOR_UPDATE_PERIODICALLY_ON_CHANGE;

    tick = 0;
    etimer_set(&et, CLOCK_CONF_SECOND * 15);

    while (1) {
        PROCESS_WAIT_EVENT();
        tick = (tick + 1) % TICKS;
        sens = stomp_sensor_processes;
        STOMP_BEGIN("tx");
        while (sens != NULL && *sens != NULL) {
            if ((*sens)->update & STOMP_SENSOR_UPDATE_PERIODICALLY) {

                if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_15s) {
                    sprintf(tmp1, "UDPATE;" UUID ";%s;%d", (*sens)->name, (*sens)->value);
                    STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                }
                if (tick % 2 == 0) {
                    if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_30s) {
                        sprintf(tmp1, "UDPATE;" UUID ";%s;%d", (*sens)->name, (*sens)->value);
                        STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                    }
                    if (tick % 4 == 0) {
                        if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_1m) {
                            sprintf(tmp1, "UDPATE;" UUID ";%s;%d", (*sens)->name, (*sens)->value);
                            STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                        }
                        if (tick == 0 || tick == 8 || tick == 16 || tick == 24 || tick == 32) {
                            if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_2m) {
                                sprintf(tmp1, "UDPATE;" UUID ";%s;%d", (*sens)->name, (*sens)->value);
                                STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                            }
                            if (tick == 0 || tick == 20) {
                                if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_5m) {
                                    sprintf(tmp1, "UDPATE;" UUID ";%s;%d", (*sens)->name, (*sens)->value);
                                    STOMP_SEND("/queue/manager", "text/plain", NULL, NULL, NULL, tmp1);
                                }
                                if (tick == 0) {
                                    if ((*sens)->periodic & STOMP_SENSOR_PERIODIC_10m) {
                                        sprintf(tmp1, "UDPATE;" UUID ";%s;%d", (*sens)->name, (*sens)->value);
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
                    sprintf(tmp1, "UDPATE;" UUID ";%s;%d", (*sens)->name, (*sens)->value);
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

#define SEMICOLON 0x3b

#define SET_ALL_LEN 7
const char _set_all[SET_ALL_LEN + 1] =
        /* "SET_ALL" */{0x53, 0x45, 0x54, 0x5f, 0x41, 0x4c, 0x4c,};

#define SET_LEN 3
const char _set[SET_LEN + 1] =
        /* "SET" */{0x53, 0x45, 0x54,};

#define GET_LEN 3
const char _get[GET_LEN + 1] =
        /* "GET" */{0x47, 0x45, 0x54,};

#define UPDATE_LEN 6
const char _update[UPDATE_LEN + 1] =
        /* "UPDATE" */{0x55, 0x50, 0x44, 0x41, 0x54, 0x45,};

void
stomp_message(char* destination, char* message_id, char* subscription, char* content_type, char* content_length, char* message) {
    int off = 0;
    char *sensor, *update, *periodic;

    PRINTA("MESSAGE: {destination=\"%s\", message_id=\"%s\", subscription=\"%s\", content_type=\"%s\", content_length=\"%s\", message=\"%s\"}.\n", destination, message_id, subscription, content_type, content_length, message);

    if (message != NULL) {
        if (*message == _set[0]) {
            while (message != NULL && *message != SEMICOLON && *message != _set_all[3])
                message += 1;
            if (message == NULL)
                return;

            if (*message == SEMICOLON) {
                message += 1;
                off = 0;
                while (message + off != NULL && *(message + off) != SEMICOLON)
                    off += 1;
                if (message + off == NULL)
                    return;
                sensor = NEW_ARRAY(char, off + 1);
                memcpy(sensor, message, off);
                message += off + 1;

                off = 0;
                while (message + off != NULL && *(message + off) != SEMICOLON)
                    off += 1;
                if (message + off == NULL)
                    return;
                update = NEW_ARRAY(char, off + 1);
                memcpy(update, message, off);
                message += off + 1;

                off = 0;
                while (message + off != NULL && *(message + off) != SEMICOLON)
                    off += 1;
                if (message + off == NULL)
                    return;
                periodic = NEW_ARRAY(char, off + 1);
                memcpy(periodic, message, off);
                message += off + 1;

                /* TODO set on specified sensor */
                PRINTA("SET;%s;%s;%s", sensor, update, periodic);
                DELETE(sensor);
                DELETE(update);
                DELETE(periodic);

            } else if (*message == _set_all[3]) {
                while (message != NULL && *message != SEMICOLON)
                    message += 1;
                if (message == NULL)
                    return;
                message += 1;

                off = 0;
                while (message + off != NULL && *(message + off) != SEMICOLON)
                    off += 1;
                if (message + off == NULL)
                    return;
                update = NEW_ARRAY(char, off + 1);
                memcpy(update, message, off);
                message += off + 1;

                off = 0;
                while (message + off != NULL && *(message + off) != SEMICOLON)
                    off += 1;
                if (message + off == NULL)
                    return;
                periodic = NEW_ARRAY(char, off + 1);
                memcpy(periodic, message, off);
                message += off + 1;

                /* TODO set on all sensors */
                PRINTA("SET_ALL;%s;%s", update, periodic);
                DELETE(update);
                DELETE(periodic);

            }
        } else if (*message == _get[0]) {
            while (message != NULL && *message != SEMICOLON)
                message += 1;
            if (message == NULL)
                return;
            message += 1;

            off = 0;
            while (message + off != NULL && *(message + off) != SEMICOLON)
                off += 1;
            if (message + off == NULL)
                return;
            sensor = NEW_ARRAY(char, off + 1);
            memcpy(sensor, message, off);
            message += off + 1;

            /* TODO get settings on specified sensor */
            PRINTA("GET;%s", sensor);
            DELETE(sensor);

        } else if (*message == _update[0]) {
            while (message != NULL && *message != SEMICOLON)
                message += 1;
            if (message == NULL)
                return;
            message += 1;

            off = 0;
            while (message + off != NULL && *(message + off) != SEMICOLON)
                off += 1;
            if (message + off == NULL)
                return;
            sensor = NEW_ARRAY(char, off + 1);
            memcpy(sensor, message, off);
            message += off + 1;
            /* TODO update value from sensor */
            PRINTA("UPDATE;%s", sensor);
            DELETE(sensor);

        } else {
            PRINTA("Unrecognized operation.\n");
        }
    }
}

void
stomp_error(char* receipt_id, char* content_type, char* content_length, char *message) {
    PRINTA("ERROR: {receipt_id=\"%s\", content_type=\"%s\", content_length=\"%s\", message=\"%s\"}.\n", receipt_id, content_type, content_length, message);
}

void
stomp_receipt(char* receipt_id) {
    PRINTA("RECEIPT: {receipt_id=\"%s\"}.\n", receipt_id);
}

void
stomp_connected(char* version, char* server, char* host_id, char* session, char* heart_beat, char* user_id) {
    PRINTA("CONNECTED: {version=\"%s\", server=\"%s\", host_id=\"%s\", session=\"%s\", heart_beat=\"%s\", user_id=\"%s\"}.\n", version, server, host_id, session, heart_beat, user_id);
}

void
stomp_received(char *buf, int len) {
    PRINTA("Unknown: {buf=\"%s\", len=%d}.\n", buf, len);
}

void
stomp_net_connected() {
    PRINTA("Connected.\n");
    process_post(&stomp_client_process, PROCESS_EVENT_CONTINUE, NULL);
}