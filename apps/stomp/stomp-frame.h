/*
 * frame.h
 *
 *  Created on: 24-03-2012
 *      Author: paoolo
 */

#ifndef FRAME_H_
#define FRAME_H_

#define _FRAME_LF 						'\n'
#define _FRAME_SEPERATOR				':'

/* Strutura przechowujaca wartosci naglowka */
struct _frame_header_t {
	/* Wskazuje na kolejne pole w naglowku */
	struct _frame_header_t *next;
	/* Przechowuje nazwe, zmiennej dlugosci */
	char *name;
	/* Przechowuje wartosc pola, zmienna dlugosc */
	char *value;
};

typedef struct _frame_header_t frame_header_t;

/* Struktura przechowujaca calosc ramki */
struct _frame_t {
	/* Przechowuje jedno z dostepnych polecen */
	char *command;
	/* Wskazuje na jednokierunkowa liste kolejnych pol naglowka */
	frame_header_t *headers;
	/* Inne dane o zmiennej dlugosci */
	char *payload;
};

typedef struct _frame_t frame_t;

/* Mozliwe polecenia przekazywane przez ramke */
#define _FRAME_CMD_SEND "SEND"
#define _FRAME_CMD_SUBSCRIBE "SUBSCRIBE"
#define _FRAME_CMD_UNSUBSCRIBE "UNSUBSCRIBE"
#define _FRAME_CMD_BEGIN "BEGIN"
#define _FRAME_CMD_COMMIT "COMMIT"
#define _FRAME_CMD_ABORT "ABORT"
#define _FRAME_CMD_ACK "ACK"
#define _FRAME_CMD_NACK "NACK"
#define _FRAME_CMD_DISCONNECT "DISCONNECT"
#define _FRAME_CMD_CONNECT "CONNECT"
#define _FRAME_CMD_STOMP "STOMP"
#define _FRAME_CMD_CONNECTED "CONNECTED"
#define _FRAME_CMD_MESSAGE "MESSAGE"
#define _FRAME_CMD_RECEIPT "RECEIPT"
#define _FRAME_CMD_ERROR "ERROR"

/* Mozliwe okreslenia naglowkow */
#define _FRAME_HEADER_ACCEPT_VERSION "accept-version"
#define _FRAME_HEADER_ACK_MODE "ack"
#define _FRAME_HEADER_AMQ_MESSAGE_TYPE "amq-msg-type"
#define _FRAME_HEADER_AUTO "auto"
#define _FRAME_HEADER_CLIENT "client"
#define _FRAME_HEADER_CLIENT_ID "client-id"
#define _FRAME_HEADER_CONTENT_LENGTH "content-length"
#define _FRAME_HEADER_CONTENT_TYPE "content-type"
#define _FRAME_HEADER_CORRELATION_ID "correlation-id"
#define _FRAME_HEADER_DESTINATION "destination"
#define _FRAME_HEADER_EXPIRATION_TIME "expires"
#define _FRAME_HEADER_HOST "host"
#define _FRAME_HEADER_ID "id"
#define _FRAME_HEADER_INDIVIDUAL "client-individual"
#define _FRAME_HEADER_LOGIN "login"
#define _FRAME_HEADER_MESSAGE_ID "message-id"
#define _FRAME_HEADER_MESSAGE_H "message"
#define _FRAME_HEADER_ORIGINAL_DESTINATION "original-destination"
#define _FRAME_HEADER_PASSCODE "passcode"
#define _FRAME_HEADER_PERSISTENT "persistent"
#define _FRAME_HEADER_PRIORITY "priority"
#define _FRAME_HEADER_RECEIPT_ID "receipt-id"
#define _FRAME_HEADER_RECEIPT_REQUESTED  "receipt"
#define _FRAME_HEADER_REDELIVERED "redelivered"
#define _FRAME_HEADER_REPLY_TO "reply-to"
#define _FRAME_HEADER_REQUEST_ID "request-id"
#define _FRAME_HEADER_RESPONSE_ID "response-id"
#define _FRAME_HEADER_SELECTOR "selector"
#define _FRAME_HEADER_SESSION "session"
#define _FRAME_HEADER_SUBSCRIPTION "subscription"
#define _FRAME_HEADER_TIMESTAMP "timestamp"
#define _FRAME_HEADER_TRANSACTION "transaction"
#define _FRAME_HEADER_TRANSFORMATION_ERROR "transformation-error"
#define _FRAME_HEADER_TRANSFORMATION "transformation"
#define _FRAME_HEADER_TYPE "type"
#define _FRAME_HEADER_USERID "JMSXUserID"

/* Usuwa naglowek ramki, wraz z zaleznoscmia */
void _frame_delete_header(frame_header_t *header);

/* Usuwa cala ramke, wraz z zaleznoscmia */
void _frame_delete_frame(frame_t *frame);

/* Tworzy nowy obiekt naglowka, uzywany przez ramke */
frame_header_t* _frame_new_header(char *name, char *value);

/* Tworzy nowy obiekt ramki */
frame_t* _frame_new_frame(char *command, frame_header_t *headers, char *payload);

/* Szuka naglowka */		
frame_header_t* _frame_find_header(char *name, frame_t *frame);

/* Importuje ramke ze strumienia znakow */
frame_t* _frame_import_frame(char *stream, frame_t *frame);

/* Eksportuje ramke do strumienia znakow */
char* _frame_export_frame(frame_t *frame);

/* Wyswietla zawartosc ramki */
void _frame_print_frame(frame_t *frame);

/* Wyswietla ramke w surowej postaci */
void _frame_print_raw_frame(frame_t *frame);

/* Wielkosc ramki surowej na podstawie przekazanej struktury */
#endif /* FRAME_H_ */
