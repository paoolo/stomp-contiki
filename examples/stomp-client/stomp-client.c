#include "stomp-memguard.h"
#include "stomp-network.h"
#include "stomp-frame.h"
#include "stomp.h"

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * main.c
 *
 * To jest demo programu klienckiego. Pokazuje, jak korzystac z
 * dostarczonych funkcji przez biblioteke obslugi polaczenia.
 * Protokolem uzywanym w polaczeniu, jest STOMP, wersja 1.1
 *
 *  Created on: 24-03-2012
 *      Author: paoolo
 *      Author: bibro
 */

PROCESS(program, "STOMP contiki client");
AUTOSTART_PROCESSES(&program);

/* Glowny program */
PROCESS_THREAD(program, ev, data) {
	PROCESS_BEGIN();

	getchar();

	/* Uzyty tryb polaczenia */
	int type = _NETWORK_TYPE_TCP;
	/* Numer portu serwera */
	int port = 61613;

#if UIP_CONF_IPV6 > 0
	char *host = "aaaa:0000:0000:0000:0000:0000:0000:0001";
	uint16_t host_ip[] = { 43690, 0, 0, 0, 0, 0, 0, 1 };
#else
	char *host = "10.1.1.100";
	uint8_t host_ip[] = { 10, 1, 1, 100 };
#endif

	/* Obiekt polaczenia z serwerem */
	network_conn_t *conn;
	/* Ramka, ktora odbiore */
	stomp_frame_t *frame;

	/* Ustawienie uIP stack */
	uip_init();

	/* Uruchomienie straznika pamieci */
	_memguard_init();

	/* Uzyskanie polaczenia z serwerem */
	conn = stomp_connect(host, host_ip, port, type, 1, "admin", "password");

	/* Detekcja bledow, poprzez testowanie wartosci zwracanej */
	if (conn != NULL) {
		/* Subskrybcja okreslonej kolejki */
		stomp_subscribe(0, "/queue/a", "client", conn);

		/* Wyslanie wiadomosci do kolejki */
		stomp_send("/queue/a", "Testowa wiadomosc, wysylana na serwer",
				conn);

		frame = _network_recv_frame(conn);
		_del_ref(frame);

		/* Wypisanie sie z kolejki */
		stomp_unsubscribe(0, conn);

		getchar();

		/* Zakonczenie pracy z serwerem */
		stomp_disconnect(0, conn);
	}

	getchar();

	PROCESS_END();
}
