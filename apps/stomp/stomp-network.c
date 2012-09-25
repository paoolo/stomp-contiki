#include "stomp-network.h"

#include "stomp-memguard.h"
#include "stomp-tools.h"
#include "stomp-frame.h"

#include "contiki-net.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * network.c
 *
 * Moduł dostarczajacy funkcjonalnosci zwiazanych z obsluga polaczenia z serwerem.
 * API tego moduly opisane jest w pliku naglowkowym, zas tutaj dostarczone sa
 * jeszcze funkcje odpowiedzialne za wykonanie tych samych operacji w zaleznosci
 * od kontekstu.
 *
 * Planowane jest tez dolozenie mechanizmu buforowania komunikatow.
 *
 *  Created on: 22-03-2012
 *      Author: paoolo
 */

/* Usuwa obiekt polaczenia, wraz z zaleznosciami */
void _network_disconnect(network_conn_t *conn) {
	/* Zamykamy polaczenie */
	uip_close();
}

/* Ustawienie obiektu typu network_conn_t. */
void __network_set_up_network_conn(struct uip_conn *sock, int type,
		int port, char *host, network_conn_t *conn) {

	/* Jesli brak obiektu polaczenia */
	if (conn == NULL) {
		conn = (network_conn_t*) _deref(_new_ref(sizeof(network_conn_t)));
	}

	/* Zapisanie wartości deskryptora polaczenia */
	conn->conn = sock;
	/* Zapisanie typu polaczenia z serwerem (TCP lub UDP) */
	conn->type = type;
	/* Zapisasanie wartosci numeru portu */
	conn->port = port;
	/* Zapisanie (poprzez skopiowanie) wartosci hosta */
	conn->host = _tools_strcpy(host);

}

/* Ustawienie obiektu typu network_conn_t */
#if UIP_CONF_IPV6 > 0
void __network_set_up_network_conn_ip(struct uip_conn *sock, int type,
		int port, uint16_t *addr, network_conn_t *conn) {
#else
void __network_set_up_network_conn_ip(struct uip_conn *sock, int type,
		int port, uint8_t *addr, network_conn_t *conn) {
#endif
	/* Jesli brak obiektu polaczenia */
	if (conn == NULL) {
		conn = (network_conn_t*) _deref(_new_ref(sizeof(network_conn_t)));
	}

	/* Zapisanie wartości deskryptora polaczenia */
	conn->conn = sock;
	/* Zapisanie typu polaczenia z serwerem (TCP lub UDP) */
	conn->type = type;
	/* Zapisasanie wartosci numeru portu */
	conn->port = port;
	/* Zapisanie adresu IP */
	conn->addr = _deref(addr);
}

/* Przeksztalcenie wartosci hex do calkowitoliczbowej
 * zalozenie jest takie, ze ciag znaku jest dlugosci cztery */
uint16_t __parse_hex(char *hex, int len) {
	/* Zmienna pomocnicza */
	int i;
	/* Wartosc, ktora bedzie zwracana */
	uint16_t ret = 0;

	/* Parsowanie ciagu znaku, wraz z walidacja wartosci */
	for (i = 0; i < len && hex != NULL && *hex != '\0'; i++) {
		if (*hex >= '0' && *hex <= '9') {
			ret = 16 * ret + (*hex - '0');
		} else if (*hex >= 'a' && *hex <= 'f') {
			ret = 16 * ret + (*hex - 'a');
		} else if (*hex >= 'A' && *hex <= 'F') {
			ret = 16 * ret + (*hex - 'A');
		}
		hex = hex + 1;
	}

	/* Zwrocenie wyniku */
	return ret;
}

/* Utworzenie struktury z adresem */
uip_ipaddr_t *__parse_address(char *addr) {
	/* Zmienne pomocnicze */
	int i;
	/* Pole adresu */
	char *chunk;

#if UIP_CONF_IPV6 > 0
	/* Zmienne pomocnicze dla adresu IPv6 */
	uint16_t *addr_ = _deref(_new_ref(sizeof(uint16_t) * 8));
	/* Pomocniczy wskaznik do tablicy */
	uint16_t *addr_ptr = addr_;
#else
	/* Zmienne pomocnicze dla adresu IPv4 */
	uint8_t *addr_ = _deref(_new_ref(sizeof(uint8_t) * 4));
	/* Pomocniczy wskaznik do tablicy */
	uint8_t *addr_ptr = addr_;
#endif

	/* Struktura adresu */
	uip_ipaddr_t *__ip_addr = _deref(_new_ref(sizeof(uip_ipaddr_t)));

#if UIP_CONF_IPV6 > 0
	for (i = 0; i < 8 && addr != NULL && *addr != '\0'; i++) {
		/* Pobranie kolejnego fragmentu adresu */
		chunk = _tools_strtok(&addr, ':');
		/* Parsowanie */
		*addr_ptr = __parse_hex(chunk, 4);
		/* Przejscie */
		addr_ptr = addr_ptr + 1;
#else
		for (i = 0; i < 4 && addr != NULL && *addr != '\0'; i++) {
			/* Pobranie kolejnego fragmentu adresu */
			chunk = _tools_strtok(&addr, '.');
			/* Parsowanie */
			*addr_ptr = atoi(chunk);
			/* Przjscie */
			addr_ptr = addr_ptr + 1;
#endif
		/* Zwolnienie pamieci */
		_del_ref(chunk);
	}

#if UIP_CONF_IPV6 > 0
	/* Uzupelnienie struktury adresu IPv6 */
	uip_ip6addr(__ip_addr, addr_[0], addr_[1], addr_[2], addr_[3],
			addr_[4], addr_[5], addr_[6], addr_[7]);
#else
	/* Uzupelnienie struktury adresu IPv4 */
	uip_ipaddr(__ip_addr, addr_[0], addr_[1], addr_[2], addr_[3]);
#endif

	return __ip_addr;
}

/* Uzyskanie polaczenia z serwerem */
network_conn_t* _network_connect(char *addr, int port, int type,
		network_conn_t *conn) {

	/* Tworzenie obiektu polaczenia, gdy go nie ma */
	if (conn == NULL) {
		conn = (network_conn_t*) _deref(_new_ref(sizeof(network_conn_t)));
	}

	/* Struktura z adresem */
	uip_ipaddr_t *ip_addr = __parse_address(addr);

	/* Zapisanie informacji o polaczeniu do struktury */
	__network_set_up_network_conn(NULL, type, port, addr, conn);

	if (type == _NETWORK_TYPE_TCP) {
		/* Otwarcie _NETWORK_TYPE_TCP gniazda */
		conn->conn = tcp_connect(ip_addr, UIP_HTONS(port), NULL);

	} else if (type == _NETWORK_TYPE_UDP) {
		/* Otwarcie _NETWORK_TYPE_UDP gniazda */
		conn->conn_udp = udp_new(ip_addr, UIP_HTONS(port), NULL);
	}

	/* Zwolnienie pamieci */
	_del_ref(ip_addr);

	/* Weryfikacja otwarcia polaczenia */
	if (conn->conn == NULL) {
		/* FIXME uzyc innego sposobu raportowania o bledzie */
		return NULL;
	}

	return conn;
}

#if UIP_CONF_IPV6 > 0
/* Uzyskanie polaczenia z serwerem */
PT_THREAD(void _network_connect_ip(uint16_t *addr, int port, int type,
		network_conn_t *conn)) {
#else
PT_THREAD(void _network_connect_ip(uint8_t *addr, int port, int type,
		network_conn_t *conn)) {
#endif
	/* Tworzenie obiektu polaczenia, gdy go nie ma */
	if (conn == NULL) {
		conn = (network_conn_t*) _deref(_new_ref(sizeof(network_conn_t)));
	}

	/* Struktura adresu */
	uip_ipaddr_t *ip_addr = _deref(_new_ref(sizeof(uip_ipaddr_t)));

#if UIP_CONF_IPV6 > 0
	/* Uzupelnienie struktury adresu IPv6 */
	uip_ip6addr(ip_addr, addr[0], addr[1], addr[2], addr[3],
			addr[4], addr[5], addr[6], addr[7]);
#else
	/* Uzupelnienie struktury adresu IPv4 */
	uip_ipaddr(ip_addr, addr[0], addr[1], addr[2], addr[3]);
#endif

	/* Zapisanie informacji o polaczeniu do struktury */
	__network_set_up_network_conn_ip(NULL, type, port, addr, conn);

	PT_BEGIN(pt);
	if (type == _NETWORK_TYPE_TCP) {
		/* Otwarcie _NETWORK_TYPE_TCP gniazda */
		conn->conn = tcp_connect(ip_addr, UIP_HTONS(port), NULL);
		PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

	} else if (type == _NETWORK_TYPE_UDP) {
		/* Otwarcie _NETWORK_TYPE_UDP gniazda */
		conn->conn_udp = udp_new(ip_addr, UIP_HTONS(port), NULL);
	}

	/* Zwolnienie pamieci */
	_del_ref(ip_addr);
}

/* Mechanizm pozwalajacy na wyslanie calej ramki,
 * prowizorycznie, dla _NETWORK_TYPE_UDP */
void __network_send_frame_udp(char *stream, int len, network_conn_t *conn) {

	if (stream == NULL || len < 0 || conn == NULL) {
		/* TODO powiadomic o blednych danych przekazaych do funkcji */
	}

	/* TODO not implemented yet, bowiem wymaga to tez zmian na serwerze, w zwiazku
	 * z utworzeniem mechanizmow kontroli przesylania danych przez sieci, przy pomocy
	 * _NETWORK_TYPE_UDP. */

}

/* Mechanizm pozwalajacy na dosylanie brakujacych elementow,
 * prowizorycznie, dla _NETWORK_TYPE_TCP */
void __network_send_frame_tcp(char *stream, int len, network_conn_t *conn) {

	printf("Send frame:\n");
	printf(stream);

	if(uip_connected()) {
		/* Wyslanie danych na serwer przy uzyciu obecnego polaczenia */
		uip_send(stream, len);
		printf("done.\n");
	} else {
		printf("fail - not connected.\n");
	}

}

/*
 * Pozwala na wysłanie ramki, podając ją jako obiekt.
 * Wymagany jest tez obiekt polaczenia z serwerem.
 */
void _network_send_frame(frame_t *frame, network_conn_t *conn) {
	/* Okresla dlugosc ramki */
	int len = 0;
	/* Strumien danych do wyslania */
	char *stream = NULL;

	/* Przygotowanie ramki do wyslania */
	stream = _frame_export_frame(frame);

	/* Pobranie wartosci dlugosci ramki do wyslania */
	/* @bibro: Zwiększone o 1 poniewaz wczesniej chyba nie liczylo 0x00 i ramka miala zla postac
	 * @paoolo: Faktycznie, zapomnialem o tym, ze strlen konczy na \0, rownoznaczne z 0x00 */
	if (stream != NULL) {
		len = strlen(stream) + 1;
	}

	/* Jesli tylko jest cos do wyslania, to .. */
	if (stream != NULL && len > 0) {
		if (conn->type == _NETWORK_TYPE_TCP) {
			/* Wysylanie po _NETWORK_TYPE_TCP */
			__network_send_frame_tcp(stream, len, conn);

		} else if (conn->type == _NETWORK_TYPE_UDP) {
			/* Wysylanie pod _NETWORK_TYPE_UDP (sic!) */
			__network_send_frame_udp(stream, len, conn);

		}
	}
}

/* Okresla wielkosc bufora odbioru danych */
#define _BUFLEN 128

/* Mechanizm pozwalajacy na odebranie ramki, do momentu napotkania
 * znaki konca ramki. Z wykorzystaniem _NETWORK_TYPE_UDP. */
char* __network_recv_frame_udp(network_conn_t *conn) {

	if (conn == NULL) {
		/* TODO powiadomic o przekazanych blednych danych */
	}

	/* TODO not implemented yet, bowiem wymaga to tez zmian na serwerze, w zwiazku
	 * z utworzeniem mechanizmow kontroli przesylania danych przez sieci, przy pomocy
	 * _NETWORK_TYPE_UDP. */
	return NULL;

}

/* Mechanizm pozwalajacy na odebranie ramki, do momentu napotkania
 * znaki konca ramki. Z wykorzystaniem _NETWORK_TYPE_TCP. */
char* __network_recv_frame_tcp(network_conn_t *conn) {
	/* Zwracany strumien danych, ciagle powiekszany o kolejne fragmenty pobrane do bufora */
	char *stream = NULL, *tmp = NULL;

	if (uip_newdata()) {
		/* Odebranie danych */
		stream = _tools_strcpy(uip_appdata);
	}

	/* FIXME might be a bug in case of recognizion the end of frame */
	while (uip_newdata()) {
		/* Laczenie danych z nowo pobranymi */
		tmp = _tools_strcat(stream, uip_appdata);
		/* Usuwanie poprzednia postac ramki */
		free(stream);
		/* Nowa ramka */
		stream = tmp;
	}

	/* Zwracmy odebrana ramke */
	return stream;
}

/*
 * Pozwala na odebranie ramki, ktora zwraca jest jako obiekt.
 * Wymagane jest podanie obiektu polaczeni z serwerem. Moze zwrocic
 * NULL, jesli napotka bledy w odbiorze danych.
 */
frame_t* _network_recv_frame(network_conn_t *conn) {
	/* Obiekt ramki, ktory zostanie zwrocony */
	frame_t *frame = NULL;
	/* Bufor na dane otrzymane z serwera */
	char *stream = NULL;
	/* Jak okreslic dlugosc ramki? */

	if (conn->type == _NETWORK_TYPE_TCP) {
		/* Odbieranie po _NETWORK_TYPE_TCP */
		stream = __network_recv_frame_tcp(conn);

	} else if (conn->type == _NETWORK_TYPE_UDP) {
		/* Odbieranie po _NETWORK_TYPE_UDP (sic!) */
		stream = __network_recv_frame_udp(conn);

	}

	if (stream != NULL) {
		/* Parsowanie strumienia wejsciowego */
		frame = _frame_import_frame(stream, NULL);

	}

	_del_ref(stream);

	return frame;
}
