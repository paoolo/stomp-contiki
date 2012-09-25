#include "stomp.h"

#include "contiki.h"
#include "contiki-net.h"

#include "stomp-memguard.h"
#include "stomp-tools.h"
#include "stomp-network.h"
#include "stomp-frame.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * stomp.c
 *
 *  Created on: 26-03-2012
 *      Author: paoolo
 *      Author: bibro
 */

#define TEXT_PLAIN "text/plain"

/*
 * Connecting
 *
 * A STOMP client initiates the stream or TCP connection to the server by
 * sending the CONNECT frame:
 *
	CONNECT
	accept-version:1.1
	host:stomp.github.org

	^@
 *
 * If the server accepts the connection attempt it will respond with a
 * CONNECTED frame:
 *
	CONNECTED
	version:1.1

	^@
 *
 * The server can reject any connection attempt. The server SHOULD respond
 * back with an ERROR frame listing why the connection was rejected and
 * then close the connection. STOMP servers MUST support clients which
 * rapidly connect and disconnect. This implies a server will likely only
 * allow closed connections to linger for short time before the connection
 * is reset. This means that a client may not receive the ERROR frame before
 * the socket is reset.
 *
 * CONNECT or STOMP Frame
 *
 * STOMP servers SHOULD handle a STOMP frame in the same manner as a CONNECT
 * frame. STOMP 1.1 clients SHOULD continue to use the CONNECT command to
 * remain backward compatible with STOMP 1.0 servers.
 *
 * Clients that use the STOMP frame instead of the CONNECT frame will only be
 * able to connect to STOMP 1.1 servers but the advantage is that a protocol
 * sniffer/discriminator will be able to differentiate the STOMP connection
 * from an HTTP connection.
 *
 * STOMP 1.1 clients MUST set the following headers:
 * * accept-version : The versions of the STOMP protocol the client supports.
 *   See Protocol Negotiation for more details.
 * * host : The name of a virtual host that the client wishes to connect to.
 *   It is recommended clients set this to the host name that the socket was
 *   established against, or to any name of their choosing. If this header
 *   does not match a known virtual host, servers supporting virtual hosting
 *   MAY select a default virtual host or reject the connection.
 *
 * STOMP 1.1 clients MAY set the following headers:
 * * login : The user id used to authenticate against a secured STOMP server.
 * * passcode : The password used to authenticate against a secured STOMP server.
 *
 * CONNECTED Frame
 *
 * STOMP 1.1 servers MUST set the following headers:
 * * version : The version of the STOMP protocol the session will be using.
 *   See Protocol Negotiation for more details.
 *
 * STOMP 1.1 servers MAY set the following headers:
 * * session : A session id that uniquely identifies the session.
 * * server : A field that contains information about the STOMP server. The
 *   field MUST contain a server-name field and MAY be followed by optional
 *   comment feilds delimited by a space character.
 *
 * The server-name field consists of a name token followed by an optional version
 * number token.

	server = name ["/" version] *(comment)

 * Example:

	server:Apache/1.3.9

 *
 */
#if UIP_CONF_IPV6 > 1
network_conn_t* stomp_connect(char* host, uint16_t *host_ip, int port, int type,
		int auth_enable, char* login, char* password) {
#else
network_conn_t* stomp_connect(char* host, uint8_t *host_ip, int port, int type,
		int auth_enable, char* login, char* password) {
#endif

	/* Obiekt polaczenia, ktory zostanie zwrocony, po pozytywnym zakonczeniu
	 * negocjacji polaczenia. Jesli nie dojdzie do pozytywnego nawiazania polaczenia,
	 * zostanie zamkniete polaczenie fizyczne (o ile jest), i zwrocony NULL. */
	network_conn_t *conn;

	/* Obiekt ramki CONNECT, jaka zostanie wyslana jako pierwsza, cele wykonania
	 * polaczenia z serwerem. */
	frame_t *connect_frame;
	/* Obiekt ramke CONNECTED lub ERROR, zwroconej przez serwer */
	frame_t *__received_frame;

	/* Przygotowanie odpowiednich naglowkow:
	 * dla wersji akceptowanych przez klienta */
	frame_header_t *__accept_version_header;
	/* dla nazwy hosta do ktorego klient chce sie podlaczyc */
	frame_header_t *__host_header;
	/* login klienta  */
	frame_header_t *__login_header;
	/* haslo klienta  */
	frame_header_t *__password_header;

	/* Utworzenie obiektu poleczenia z serwerem, ktory to obiekt zostanie zwrocony */
	conn = _network_connect_ip(host_ip, port, type, NULL);

	/* Testowanie, czy nastapilo fizyczne polaczenie z serwerem */
	if(conn == NULL) {
	    printf("_error_ _network_connect\n");
		/* TODO powiadomic o bledzie polaczenia */
		return NULL;
	}

	/* Ustawienie naglowkow */
	__accept_version_header = _frame_new_header(_FRAME_HEADER_ACCEPT_VERSION, "1.1");
	__host_header = _frame_new_header(_FRAME_HEADER_HOST, host);

	/* W przypadku zastosowania autentykacji */
	if (auth_enable > 0) {
		__login_header = _frame_new_header(_FRAME_HEADER_LOGIN, login);
		__password_header = _frame_new_header(_FRAME_HEADER_PASSCODE, password);
	}

	/* Utworzenie listy naglowkow */
	if (auth_enable > 0) {
		__login_header->next =  __password_header;
		__password_header->next = __host_header;
	}
	__host_header->next = __accept_version_header;
	
	/* Utworzenie ramki CONNECT */
	if (auth_enable > 0) {
		connect_frame = _frame_new_frame(_FRAME_CMD_CONNECT, __login_header, "\n");
	} else {
		connect_frame = _frame_new_frame(_FRAME_CMD_CONNECT, __host_header, "\n");
	}

	/* Wyslanie ramki CONNECT */
	_network_send_frame(connect_frame, conn);

	/* Zwolnienie zasobow wykorzystywanych przez ramke */
	_frame_delete_frame(connect_frame);

	/* Oczekiwanie na odebranie ramki od serwera. Gdy serwer znajdzie, wsrod obslugiwanych
	 * przez klienta wersje, odsyla ramke CONNECTED. Jesli nie, to ramke ERROR. */

	/* Odebranie ramki CONNECTED albo ERROR */
	__received_frame = _network_recv_frame(conn);
	
	/* Sprawdzamy, czy nie dostalismy ramki ERROR */
	if (__received_frame != NULL && strcmp(__received_frame->command, _FRAME_CMD_ERROR) == 0){
		/* Odszukanie pola naglowka message */
		frame_header_t *__frame_header_t = _frame_find_header(
				_FRAME_HEADER_MESSAGE_H, __received_frame);

        printf("_error_ ERROR frame message %s\n", __frame_header_t->value);
        /* TOOD powiadomic o otrzymaniu ramki ERROR */

		/* Zwolnienie zasobow */
		_frame_delete_frame(__received_frame);

		/* FIXME sygnalizacja wystapienia bledu */
		return NULL;
	} else {
		/* Zwolnienie zasobow */
		_frame_delete_frame(__received_frame);

		/* Zwracmy obiekt polaczenia */
		return conn;
	}
}

/*
 * SEND
 *
 * The SEND frame sends a message to a destination in the messaging system.
 * It has one REQUIRED header, destination, which indicates where to send
 * the message. The body of the SEND frame is the message to be sent. For example:
 *
	SEND
	destination:/queue/a
	content-type:text/plain

	hello queue a
	^@
 *
 * This sends a message to a destination named /queue/a. Note that STOMP treats
 * this destination as an opaque string and no delivery semantics are assumed by
 * the name of a destination. You should consult your STOMP server's documentation
 * to find out how to construct a destination name which gives you the delivery
 * semantics that your application needs.
 *
 * The reliability semantics of the message are also server specific and will depend
 * on the destination value being used and the other message headers such as the
 * transaction header or other server specific message headers.
 *
 * SEND supports a transaction header which allows for transactional sends.
 *
 * SEND frames SHOULD include a content-length header and a content-type header if
 * a body is present.
 *
 * An application MAY add any arbitrary user defined headers to the SEND frame. User
 * defined headers are typically used to allow consumers to filter messages based on
 * the application defined headers using a selector on a SUBSCRIBE frame. The user
 * defined headers MUST be passed through in the MESSAGE frame.
 *
 * If the sever cannot successfully process the SEND frame for any reason, the server
 * MUST send the client an ERROR frame and disconnect the client.
 *
 */
void stomp_send(char *__dest, char *__value, network_conn_t *__conn) {

	/* Utworzenie ramki, ktora bedzie wysylana na serwer */
	frame_t *__frame;

	/* Przygotowanie odpowiednich naglowkow:
	 * dla destination */
	frame_header_t *__dest_header;
	/* dla content-length */
	frame_header_t *__content_length_header;
	/* dla content-type */
	frame_header_t *__content_type_header;

	/* Bufor na wartosc dlugosci danych */
	char __len_string[4];
	/* pobranie dlugosc przesylanej tresci */
	int __len_int = strlen(__value);

	memset(__len_string, 0, sizeof(char) * 4);
	sprintf(__len_string, "%d", __len_int);

	/* Ustawienie naglowkow */
	__dest_header = _frame_new_header(_FRAME_HEADER_DESTINATION, __dest);
	__content_length_header = _frame_new_header(_FRAME_HEADER_CONTENT_LENGTH, __len_string);
	__content_type_header = _frame_new_header(_FRAME_HEADER_CONTENT_TYPE, TEXT_PLAIN);

	/* Ustawienie listy pol naglowkow */
	__dest_header->next = __content_length_header;
	__content_length_header->next = __content_type_header;

	/* Ustawienie typu ramki, naglowkow i payloadu */
	__frame = _frame_new_frame(_FRAME_CMD_SEND, __dest_header, __value);

	/* Wyslanie ramki */
	_network_send_frame(__frame, __conn);

	/* Zwolnienie zasobow */
	_frame_delete_frame(__frame);

	/* FIXME obsluga ramki z serwera o komunikacie bledu, czyli ERROR
	 * oraz obslugi zakonczenia polaczenia z klientem. */
}

/*
 * SUBSCRIBE
 *
 * The SUBSCRIBE frame is used to register to listen to a given destination. Like
 * the SEND frame, the SUBSCRIBE frame requires a destination header indicating
 * the destination to which the client wants to subscribe. Any messages received on
 * the subscribed destination will henceforth be delivered as MESSAGE frames from the
 * server to the client. The ack header controls the message acknowledgement mode.
 *
 * Example:

	SUBSCRIBE
	id:0
	destination:/queue/foo
	ack:client

	^@
 *
 * If the sever cannot successfully create the subscription, the server MUST send
 * the client an ERROR frame and disconnect the client.
 *
 * STOMP servers MAY support additional server specific headers to customize the
 * delivery semantics of the subscription. Consult your server's documentation for
 * details.
 *
 * UWAGA: odnośnie __id_int, zgodnie ze specyfikacją STOMP 1.1: SUBSCRIBE id Header
 *
 * An id header MUST be included in the frame to uniquely identify the subscription
 * within the STOMP connection session. Since a single connection can have multiple
 * open subscriptions with a server, the id header allows the client and server to
 * relate subsequent ACK, NACK or UNSUBSCRIBE frames to the original subscription.
 *
 * Odnośnie __ack_mode_string, zgodnie ze specyfikacją STOMP 1.1: SUBSCRIBE ack Header
 *
 * The valid values for the ack header are auto, client, or client-individual. If
 * the header is not set, it defaults to auto.
 *
 * When the the ack mode is auto, then the client does not need to send the server
 * ACK frames for the messages it receives. The server will assume the client has
 * received the message as soon as it sends it to the the client. This acknowledgment
 * mode can cause messages being transmitted to the client to get dropped.
 *
 * When the the ack mode is client, then the client MUST send the server ACK frames for
 * the messages it processes. If the connection fails before a client sends an ACK for
 * the message the server will assume the message has not been processed and MAY redeliver
 * the message to another client. The ACK frames sent by the client will be treated as
 * a cumulative ACK. This means the ACK operates on the message specified in the ACK frame
 * and all messages sent to the subscription before the ACK-ed message.
 *
 * When the the ack mode is client-individual, the ack mode operates just like the client
 * ack mode except that the ACK or NACK frames sent by the client are not cumulative. This
 * means that an ACK or NACK for a subsequent message MUST NOT cause a previous message to
 * get acknowledged.
 *
 */
void stomp_subscribe(int __id, char *__dest, char *__ack_mode, network_conn_t *__conn) {

	/* Utworzenie ramki, ktora bedzie wysylana na serwer */
	frame_t *__frame;

	/* Przygotowanie odpowiednich naglowkow:
	 * dla identyfikatora */
	frame_header_t *__id_header;
	/* dla destination, czyli kolejki, do ktorej klient chce
	 * sie przypisac */
	frame_header_t *__desination_header;
	/* dla pola ack */
	frame_header_t *__ack_header;

	char _id_string[4];
	memset(_id_string, 0, sizeof(char) * 4);
	sprintf(_id_string, "%d", __id);

	/* Utworzenie kolejnych naglowkow ramki */
	__id_header = _frame_new_header(_FRAME_HEADER_ID, _id_string);
	__desination_header = _frame_new_header(_FRAME_HEADER_DESTINATION, __dest);
	__ack_header = _frame_new_header(_FRAME_HEADER_ACK_MODE, __ack_mode);

	/* Utworzenie listy naglowkow */
	__id_header->next = __desination_header;
	__desination_header->next = __ack_header;

	/* Ustawienie typu ramki, pol naglowka i payloadu */
	__frame = _frame_new_frame(_FRAME_CMD_SUBSCRIBE, __id_header, "\n");

	/* Wyslanie ramki */
	_network_send_frame(__frame, __conn);

	/* Zwolnienie zasobow */
	_frame_delete_frame(__frame);

	/* FIXME obsluga ramki z serwera o komunikacie bledu, czyli ERROR
	 * oraz obslugi zakonczenia polaczenia z klientem. Po zapisaniu sie
	 * na dany kanal moze te byc odsylana cala zawartosc kanalu do klienta,
	 * celem zapoznania sie z nim */
}

/*
 * UNSUBSCRIBE
 *
 * The UNSUBSCRIBE frame is used to remove an existing subscription. Once the subscription
 * is removed the STOMP connections will no longer receive messages from that destination.
 * It requires that the id header matches the id value of previous SUBSCRIBE operation. Example:
 *
	UNSUBSCRIBE
	id:0

	^@
 *
 */
void stomp_unsubscribe(int __id, network_conn_t *__conn) {

	/* Utworzenie ramki, ktora bedzie wysylana na serwer */
	frame_t *__frame;

	/* Przygotowanie odpowiednich naglowkow:
	 * dla identyfikatora */
	frame_header_t *__id_header;

	char _id_string[4];
	memset(_id_string, 0, sizeof(char) * 4);
	sprintf(_id_string, "%d", __id);

	/* Ustawienie wartosci naglowka dla id */
	__id_header = _frame_new_header(_FRAME_HEADER_ID, _id_string);

	/* Ustawienie typu ramki, pol naglowka i payloadu */
	__frame = _frame_new_frame(_FRAME_CMD_UNSUBSCRIBE, __id_header, "\n");

	/* Wyslanie ramki */
	_network_send_frame(__frame, __conn);

	/* Zwolnienie zasobow */
	_frame_delete_frame(__frame);

	/* FIXME czy tutaj jest potrzeba obslugi ERROR wiadomosci od serwera? */
}

/*
 * Funkcja pomocnicza, uzywana wew. biblioteki, do wyslania ramek NACK/ACK.
 * Jedyna ich roznica jest stosowany typ ramki, wysylanej na serwera.
 */
void _stomp_nack_ack(char *_frame_cmd, int __subscription, int __message_id,
		char *__transaction, network_conn_t *__conn) {

	/* Utworzenie ramki, ktora bedzie wysylana na serwer */
	frame_t *__frame;

	/* Przygotowanie odpowiednich naglowkow:
	 * dla identyfikatora */
	frame_header_t *__subscription_header;
	/* dla message id */
	frame_header_t *__message_id_header;
	/* dla transaction */
	frame_header_t *__transaction_header;

	/* String z numeru subskrycji */
	char __subscription_string[4];
	/* String z numeru wiadomości */
	char __message_id_string[4];

	/* Parsowanie int'ów do stringa'ów */
	memset(__subscription_string, 0, sizeof(char) * 4);
	memset(__message_id_string, 0, sizeof(char) * 4);
	sprintf(__subscription_string, "%d", __subscription);
	sprintf(__message_id_string, "%d", __message_id);

	/* Przygotowanie pol naglowkow */
	__subscription_header = _frame_new_header(_FRAME_HEADER_ID, __subscription_string);
	__message_id_header = _frame_new_header(_FRAME_HEADER_MESSAGE_ID, __message_id_string);
	__transaction_header = _frame_new_header(_FRAME_HEADER_TRANSACTION, __transaction);

	/* Utworzenie listy pol naglowkow */
	__subscription_header->next = __message_id_header;
	__message_id_header->next = __transaction_header;

	/* Utworzenie obiektu ramki */
	__frame = _frame_new_frame(_frame_cmd, __subscription_header, "\n");

	/* Wyslanie ramki */
	_network_send_frame(__frame, __conn);

	/* Zwolnienie zasobow */
	_frame_delete_frame(__frame);

	/* FIXME czy tutaj jest potrzeba obslugi ERROR wiadomosci od serwera? */
}

/*
 * ACK
 *
 * ACK is used to acknowledge consumption of a message from a subscription using
 * client or client-individual acknowledgment. Any messages received from such a
 * subscription will not be considered to have been consumed until the message has
 * been acknowledged via an ACK or a NACK.
 *
 * ACK has two REQUIRED headers: message-id, which MUST contain a value matching
 * the message-id for the MESSAGE being acknowledged and subscription, which MUST
 * be set to match the value of the subscription's id header. Optionally, a
 * transaction header MAY be specified, indicating that the message acknowledgment
 * SHOULD be part of the named transaction.
 *
	ACK
	subscription:0
	message-id:007
	transaction:tx1

	^@
 *
 */
void stomp_ack(int __subscription, int __message_id, char *__transaction, network_conn_t *__conn) {

	/* Delegacja wywolania, z dodaniem parametru okreslajacego typ wysylanej wiadomosci */
	_stomp_nack_ack(_FRAME_CMD_ACK, __subscription, __message_id, __transaction, __conn);
}

/*
 * NACK
 *
 * NACK is the opposite of ACK. It is used to tell the server that the client did
 * not consume the message. The server can then either send the message to a different
 * client, discard it, or put it in a dead letter queue. The exact behavior is server
 * specific.
 *
 * NACK takes the same headers as ACK: message-id (mandatory), subscription (mandatory)
 * and transaction (OPTIONAL).
 *
 * NACK applies either to one single message (if the subscription's ack mode is
 * client-individual) or to all messages sent before and not yet ACK'ed or NACK'ed.
 *
 */
void stomp_nack(int __subscription, int __message_id, char *__transaction, network_conn_t *__conn) {

	/* Delegacja wywolania, z dodaniem parametru okreslajacego typ wysylanej wiadomosci */
	_stomp_nack_ack(_FRAME_CMD_NACK, __subscription, __message_id, __transaction, __conn);
}

/*
 * Funkcja pomocnicza, uzywana wew. biblioteki, do wyslania ramek BEGIN/COMMIT/ABORT,
 * uzywanych przy sterowaniu transakcja. Jedyna roznica pomiedzy tymi typami ramek jest
 * komenda ramki.
 */
void _stomp_begin_commit_abort(char *__frame_cmd, char *__transaction, network_conn_t *__conn) {

	/* Utworzenie ramki, ktora bedzie wysylana na serwer */
	frame_t *__frame;

	/* Przygotowanie odpowiednich naglowkow:
	 * dla transaction */
	frame_header_t *__transaction_header;

	/* Utworzenie pola naglowka */
	__transaction_header = _frame_new_header(_FRAME_HEADER_TRANSACTION, __transaction);

	/* Utworzenie obiektu ramki */
	__frame = _frame_new_frame(__frame_cmd, __transaction_header, "\n");

	/* Wyslanie ramki */
	_network_send_frame(__frame, __conn);

	/* Zwolnienie zasobow */
	_frame_delete_frame(__frame);

	/* FIXME czy tutaj jest potrzeba obslugi ERROR wiadomosci od serwera? */
}

/*
 * BEGIN
 *
 * BEGIN is used to start a transaction. Transactions in this case apply to sending and
 * acknowledging - any messages sent or acknowledged during a transaction will be handled
 * atomically based on the transaction.
 *
	BEGIN
	transaction:tx1

	^@
 *
 * The transaction header is REQUIRED, and the transaction identifier will be used for
 * SEND, COMMIT, ABORT, ACK, and NACK frames to bind them to the named transaction.
 *
 * Any started transactions which have not been committed will be implicitly aborted if
 * the client sends a DISCONNECT frame or if the TCP connection fails for any reason.
 *
 */
void stomp_begin(char *__transaction, network_conn_t *__conn) {

	/* Delegacja wywolania, z dodaniem parametru okreslajacego typ wysylanej wiadomosci */
	_stomp_begin_commit_abort(_FRAME_CMD_BEGIN, __transaction, __conn);
}

/*
 * COMMIT
 *
 * COMMIT is used to commit a transaction in progress.
 *
	COMMIT
	transaction:tx1

	^@
 *
 * The transaction header is REQUIRED and MUST specify the id of the transaction to commit!
 *
 */
void stomp_commit(char *__transaction, network_conn_t *__conn) {

	/* Delegacja wywolania, z dodaniem parametru okreslajacego typ wysylanej wiadomosci */
	_stomp_begin_commit_abort(_FRAME_CMD_COMMIT, __transaction, __conn);
}

/*
 * ABORT
 *
 * ABORT is used to roll back a transaction in progress.
 *
	ABORT
	transaction:tx1

	^@
 *
 * The transaction header is REQUIRED and MUST specify the id of the transaction to abort!
 *
 */
void stomp_abort(char *__transaction, network_conn_t *__conn) {

	/* Delegacja wywolania, z dodaniem parametru okreslajacego typ wysylanej wiadomosci */
	_stomp_begin_commit_abort(_FRAME_CMD_ABORT, __transaction, __conn);
}

/*
 * DISCONNECT
 *
 * A client can disconnect from the server at anytime by closing the socket but there is
 * no guarantee that the previously sent frames have been received by the server. To do a
 * graceful shutdown, where the client is assured that all previous frames have been received
 * by the server, the client SHOULD:
 *
 * 1. send a DISCONNECT frame with a receipt header set. Example:
 *
	DISCONNECT
	receipt:77
	^@
 *
 * 2. wait for the RECEIPT frame response to the DISCONNECT. Example:
 *
	RECEIPT
	receipt-id:77
	^@
 *
 * 3. close the socket.
 *
 * Clients MUST NOT send any more frames after the DISCONNECT frame is sent.
 *
 */
void stomp_disconnect(int __receipt, network_conn_t *__conn) {

	/* Ramka, ktora zostanie wykorzystana do wyslania komunikatu o rozlaczeniu */
	frame_t *__frame;

	/* Przygotowanie odpowiednich naglowkow:
	 * dla receipt */
	frame_header_t *__receipt_header;

	/* Konwersja zmiennej int na string */
	char __receipt_string[4];
	memset(__receipt_string, 0, sizeof(char) * 4);
	sprintf(__receipt_string, "%04d", __receipt);

	/* Utworzenie pola naglowka */
	__receipt_header = _frame_new_header(_FRAME_HEADER_RECEIPT_REQUESTED, __receipt_string);

	/* Utworzenie ramki */
	__frame = _frame_new_frame(_FRAME_CMD_DISCONNECT, __receipt_header, "\n");

	/* Wyslanie ramki */
	_network_send_frame(__frame, __conn);

	/* Zwolnienie zasobow */
	_frame_delete_frame(__frame);

	/* Oczekiwanie na ramke RECEIPT i po niej nastepuje rozlaczenie fizyczne
	 * FIXME bug w tym miejscu polega na tym, ze nie powinno sie korzystac z warstwy
	 * network w stompie, bowiem tutaj oczekujmey ramki zakonczenia, a mozemy
	 * dostac ramke z wiadomoscia i potrzeba pominac nam te ramki, zas nie bedziemy
	 * robili tego w stompie, tylko gdzies pomiedzy, m.in. w tzw. buforze ramek */
	__frame = _network_recv_frame(__conn);

	/* Wyszukwanie naglowka z receipt id */
	__receipt_header = _frame_find_header(_FRAME_HEADER_RECEIPT_ID, __frame);

	/* Weryfikacja ramki */
	if(strcmp(__frame->command, _FRAME_CMD_RECEIPT) == 0 && __receipt_header != NULL
			&& strcmp(__receipt_header->value, __receipt_string) == 0) {
		fprintf(stderr, "_debug_ zakonczono poprawnie\n");
	} else {
		fprintf(stderr, "_debug_ zakonczono niepoprawnie\n");
	}

	/* Zwalniamy ramke */
	_del_ref(__frame);

	/* Zamykniecie fizyczne polaczenia */
	_network_disconnect(__conn);
}
