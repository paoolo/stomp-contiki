#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
void example1_init(void) {
    uip_listen(UIP_HTONS(1234));
}

void example1_app(void) {
    if (uip_newdata() || uip_rexmit()) {
        uip_send("ok\n", 3);
    }
}

static struct psock ps;
static char buffer[100];
static int t;

const char connect[] = {0x43, 0x4f, 0x4e, 0x4e, 0x45, 0x43, 0x54, 0xa, 0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2d, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x3a, 0x31, 0x2e, 0x31, 0xa, 0x68, 0x6f, 0x73, 0x74, 0x3a, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0xa, 0x6c, 0x6f, 0x67, 0x69, 0x6e, 0x3a, 0x61, 0x64, 0x6d, 0x69, 0x6e, 0xa, 0x70, 0x61, 0x73, 0x73, 0x63, 0x6f, 0x64, 0x65, 0x3a, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0xa, 0xa, 0x0,};
const char send[] = {0x53, 0x45, 0x4e, 0x44, 0xa, 0x64, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x2f, 0x71, 0x75, 0x65, 0x75, 0x65, 0x2f, 0x61, 0xa, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x74, 0x79, 0x70, 0x65, 0x3a, 0x74, 0x65, 0x78, 0x74, 0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e, 0xa, 0xa, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x71, 0x75, 0x65, 0x75, 0x65, 0x20, 0x61, 0xa, 0x0,};
const char disconnect[] = {0x44, 0x49, 0x53, 0x43, 0x4f, 0x4e, 0x4e, 0x45, 0x43, 0x54, 0xa, 0x72, 0x65, 0x63, 0x65, 0x69, 0x70, 0x74, 0x3a, 0x37, 0x37, 0xa, 0xa, 0x0,};

#define PSOCK_SEND_STR_NULL(psock, str)      		\
  PT_WAIT_THREAD(&((psock)->pt), psock_send(psock, (uint8_t *)str, strlen(str)+1))

static
PT_THREAD(handle_connection(struct psock *p)) {
    PSOCK_BEGIN(p);

    PSOCK_SEND_STR_NULL(p, connect);
    // PSOCK_READTO(p, '\0');
    // printf("Got: %s", buffer);

    PSOCK_SEND_STR_NULL(p, send);
    // PSOCK_READTO(p, '\0');
    // printf("Got: %s", buffer);

    PSOCK_SEND_STR_NULL(p, disconnect);
    // PSOCK_READTO(p, '\0');
    // printf("Got: %s", buffer);

    PSOCK_END(p);
}

PROCESS(program, "Hello-uIP");
AUTOSTART_PROCESSES(&program);

PROCESS_THREAD(program, ev, data) {
    static struct etimer et;

    PROCESS_BEGIN();

    t = 1;

    getchar();

    etimer_set(&et, CLOCK_CONF_SECOND * 3);

    printf("Start..\n");

    uip_ipaddr_t *ip_addr;

    ip_addr = (uip_ipaddr_t*) malloc(sizeof (uip_ipaddr_t));

    uip_ipaddr(ip_addr, 10, 1, 1, 100);
    tcp_connect(ip_addr, UIP_HTONS(61613), NULL);

    printf("Connecting...\n");
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

    if (uip_aborted() || uip_timedout() || uip_closed()) {
        printf("Could not establish connection\n");

    } else if (uip_connected()) {
        printf("Connected\n");

        PSOCK_INIT(&ps, buffer, sizeof (buffer));

        do {
            handle_connection(&ps);
            PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
        } while (!(uip_closed() || uip_aborted() || uip_timedout()));

        printf("\nConnection closed.\n");
    }

    PROCESS_END();
}

 */

int
addr[] = {0xaaaa, 0, 0, 0, 0, 0, 0, 1};

uip_ipaddr_t
ipaddr;

int
port = 61613;

struct uip_udp_conn
*conn;

char
*str;

int
state;

char
buf[] = {0x40, 0x41, 0x42, 0x43, 0x44,};

int
len = 6;

PROCESS(program_UDP, "Hello-UDP");
AUTOSTART_PROCESSES(&program_UDP);

/* Glowny program */
PROCESS_THREAD(program_UDP, ev, data) {
    static struct etimer et;

    PROCESS_BEGIN();

#if UIP_CONF_IPV6 > 0
    uip_ip6addr(&ipaddr, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
#else
    uip_ipaddr(&ipaddr, addr[0], addr[1], addr[2], addr[3]);
#endif

    conn = udp_new(&ipaddr, UIP_HTONS(port), &state);
    udp_bind(conn, UIP_HTONS(port + 1));
    printf("Binded\n");

    etimer_set(&et, CLOCK_CONF_SECOND * 3);

    while (1) {
        PROCESS_WAIT_EVENT();
        if (uip_newdata()) {
            str = uip_appdata;
            str[uip_datalen()] = '\0';
            printf("Received: '%s'\n", str);
        }
        etimer_reset(&et);
    }

    PROCESS_END();
}