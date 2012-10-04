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

struct stomp_state {
    unsigned char flags;
    char *text;
    uint16_t textlen;
    uint16_t sentlen;
};

typedef struct stomp_state stomp_state_t;

void stomp_app(void *s);

/* Wykonuje polaczenie fizyczne i logiczne ze wskazanym serwerem. Dokonuje 
 * negocjacji co do stosowanej wersji protokoly STOMP. W przypadku niepowodzenia, 
 * zwraca NULL. */
#if UIP_CONF_IPV6 > 0
network_conn_t* stomp_connect(char *host, int port, int type,
		int auth_enabled, char* login, char* password);
#else
network_conn_t* stomp_connect(char *host, int port, int type,
		int auth_enabled, char* login, char* password);
#endif

stomp_state_t* stomp_connect(stomp_state_t *state, uip_ipaddr_t *adrr, 
        uint16_t port);

/* Callback dla ustanowionego polaczenia */
void stomp_connected(stomp_state_t *state);

/* Wysyla na wskazana kolejke wiadomosc, poprzez okreslone polaczenie z serwerem. */
void stomp_send(char *dest, char *value, char *transaction, network_conn_t *conn);

/* Callback dla wyslanej wiadomosci */
void stomp_sent(stomp_state_t *state);

/* Dokonuje subskrypcji danego kanalu. Wymagane jest podanie identyfikatora
 * subskrypcji (w obrębie klienta) */
void stomp_subscribe(int id, char *dest, char *ack_mode, network_conn_t *conn);

/* Callback dla dokonanej subskrypcji */
void stomp_subscribed(stomp_state_t *state);

/* Anuluje subskrypcje kanału wczesniej wybranego do subskrypcji. Wymagane jest
 * podanie identyfikatora wybranego przy dokonywaniu subskrypcji. */
void stomp_unsubscribe(int id, network_conn_t *conn);

/* Callback dla dokonanego wypisania sie z subskrypcji danego kanalu */
void stomp_unsubscribed(stomp_state_t *state);

/* Ramka potwierdzajaca odebranie wiadomosci z kanalu przez klienta. Wysylana
 * na serwer, po odebraniu wiadomosci, tylko wtedy wysylana, gdy klient ustawil
 * subskrypcje w trybie akceptacji client lub client-individual. Wymagane jest
 * podanie jakiego kanalu dotyczy potwierdzenie i ktorej wiadomosci. */
void stomp_ack(int subscription, int message_id, char *transaction, 
        network_conn_t *conn);

/* Callback dla dokonanej akceptacji otrzymanych lub otrzymanej wiadmosci */
void stomp_acked(stomp_state_t *state);

/* Podobnie, jak w przypadku ramki potwierdzajacej, przy przy czym, tutaj 
 * stwierdzamy niepowodzenie odbioru wiadomosci z kanalu. */
void stomp_nack(int subscription, int message_id, char *transaction, 
        network_conn_t *conn);

/* Callback dla dokonanej nie-akceptacji otrzymanej wiadomosci */
void stomp_nacked(stomp_state_t *state);

/* Rozpoczecie bloku transakcji operacji klienta na serwerze */
void stomp_begin(char *transaction, network_conn_t *conn);

/* Callback dla dokonanego rozpoczecie transakcji */
void stomp_begined(stomp_state_t *state);

/* Uznanie zmian za poprawne */
void stomp_commit(char *transaction, network_conn_t *conn);

/* Callback dla dokonanego zatwierdzenia transakcji */
void stomp_commited(stomp_state_t *state);

/* Anulowanie wszystkich operacji od momentu rozpoczecie transakcji */
void stomp_abort(char *transaction, network_conn_t *conn);

/* Callback dla dokonanego przerwania transakcji */
void stomp_aborted(stomp_state_t *state);

/* Rozlaczenie sie z serwerem. W przypadku istnienia rozpoczetych transakcji, 
 * nastepuje ich przerwanie. */
void stomp_disconnect(int receipt, network_conn_t *conn);

/* Callback dla dokonanego rozlaczenia. */
void stomp_disconnected();

/* Callback dla przekroczenia czasu oczekiwania na polaczeniu. */
void stomp_timedout();

/* Callback dla odebrania nowych danych z serwera. */
void stomp_newdata(char *data, uint16_t lenght);

#endif /* STOMP_H_ */
