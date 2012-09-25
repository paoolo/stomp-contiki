#include "contiki.h"

#include "stomp-network.h"
#include "stomp-frame.h"

/*
 * stomp.h
 *
 *  Created on: 26-03-2012
 *      Author: paoolo
 *      Author: bibro
 */

#ifndef STOMP_H_
#define STOMP_H_

/* Wykonuje polaczenie fizyczne i logiczne ze wskazanym serwerem
 * Dokonuje negocjacji co do stosowanej wersji protokoly STOMP.
 * W przypadku niepowodzenia, zwraca NULL. */
#if UIP_CONF_IPV6 > 0
network_conn_t* stomp_connect(char *host, uint16_t *host_ip, int port, int type,
		int auth_enabled, char* login, char* password);
#else
network_conn_t* stomp_connect(char *host, uint8_t *host_ip, int port, int type,
		int auth_enabled, char* login, char* password);
#endif

/* Wysyla na wskazana kolejke wiadomosc, poprzez okreslone polaczenie z serwerem. */
void stomp_send(char *dest, char *value,
		network_conn_t *conn);

/* Dokonuje subskrypcji danego kanalu. Wymagane jest podanie identyfikatora
 * subskrypcji (w obrębie klienta) */
void stomp_subscribe(int id, char *dest, char *ack_mode,
		network_conn_t *conn);

/* Anuluje subskrypcje kanału wczesniej wybranego do subskrypcji. Wymagane jest
 * podanie identyfikatora wybranego przy dokonywaniu subskrypcji. */
void stomp_unsubscribe(int id, network_conn_t *conn);

/* Ramka potwierdzajaca odebranie wiadomosci z kanalu przez klienta. Wysylana
 * na serwer, po odebraniu wiadomosci, tylko wtedy wysylana, gdy klient ustawil
 * subskrypcje w trybie akceptacji client lub client-individual. Wymagane jest
 * podanie jakiego kanalu dotyczy potwierdzenie i ktorej wiadomosci. */
void stomp_ack(int subscription, int message_id, char *transaction,
		network_conn_t *conn);

/* Podobnie, jak w przypadku ramki potwierdzajacej, przy przy czym, tutaj stwierdzamy
 * niepowodzenie odbioru wiadomosci z kanalu. */
void stomp_nack(int subscription, int message_id, char *transaction,
		network_conn_t *conn);

/* Rozpoczecie bloku transakcji operacji klienta na serwerze */
void stomp_begin(char *transaction, network_conn_t *conn);

/* Uznanie zmian za poprawne */
void stomp_commit(char *transaction, network_conn_t *conn);

/* Anulowanie wszystkich operacji od momentu rozpoczecie transakcji */
void stomp_abort(char *transaction, network_conn_t *conn);

/* Rozlaczenie sie z serwerem */
void stomp_disconnect(int receipt, network_conn_t *conn);

#endif /* STOMP_H_ */
