#include "stomp-frame.h"

#include "contiki-net.h"

/*
 * network.h
 *
 *  Created on: 24-03-2012
 *      Author: paoolo
 */

#ifndef NETWORK_H_
#define NETWORK_H_

/* Okreslenie protokolu wykorzystywanego w komunikacji, podawane
 * jako type do funkcji _connect() */
#define _NETWORK_TYPE_TCP 0
#define _NETWORK_TYPE_UDP 1

/* Struktura przechowujaca informacje o polaczeniu */
struct _network_conn_t {
	/* Obiekt polaczenia TCP */
	struct uip_conn *conn;
	/* Obiekt polaczenia UDP */
	struct uip_udp_conn *conn_udp;
	/* Typ polaczenia TCP lub UDP */
	int type;
	/* Port uzywany w polaczeniu */
	int port;
	/* Nazwa hosta, z ktorym jest polaczenie */
	char *host;
	/* Adres IP, w postaci zrozumiałej dla contiki */
#if UIP_CONF_IPV6 > 0
	uint16_t *addr;
#else
	uint8_t *addr;
#endif

};

typedef struct _network_conn_t network_conn_t;

/* Zamyka polaczenia, przy okazji usuwa obiekt polaczenia, wraz z zaleznosciami */
void _network_disconnect(network_conn_t *conn);

/* Uzyskania polaczenia z serwerem, wymagane jest podanie nazwy serwera
 * do ktore sie podlaczamy, numer portu, wskazanie z ktorego protokolu
 * korzystamy oraz (opcjonalnie) obiekt polaczenia. Zwracany jest
 * wypelniony obiekt polaczenia (w razie powodzenia) lub NULL (w razie
 * niepowodzenia). */
network_conn_t* _network_connect(char *host, int port, int type,
		network_conn_t *conn);

#if UIP_CONF_IPV6 > 0
/* Uzyskanie polaczenia z serwerem, podanie adresu IP w postaci numerycznej */
void _network_connect_ip(uint16_t *addr, int port, int type,
		network_conn_t *conn);
#else
void _network_connect_ip(uint8_t *addr, int port, int type,
		network_conn_t *conn);
#endif

/* Wyslanie ramki, wymagane jest podanie obiektu ramki do wyslania oraz
 * obiekt polaczenia, przez ktore chcemy wyslac ramke. */
void _network_send_frame(frame_t *frame, network_conn_t *conn);

/* Odebranie ramki, wymagane jest podanie obiektu polaczenia, skad
 * moze nadejsc ramka. W przypadku powodzenia zostanie ona zwrocona.
 * W przypadku bledu, zostanie zwrocony NULL. */
frame_t* _network_recv_frame(network_conn_t *conn);

#endif /* NETWORK_H_ */
