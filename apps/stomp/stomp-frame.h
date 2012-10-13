#ifndef FRAME_H_
#define FRAME_H_

#define STOMP_NULL 0x00
#define STOMP_COLON 0x3a
#define STOMP_NEW_LINE 0x0a

#define STOMP_HEADER_NAME_LEN 21
#define STOMP_HEADER_VALUE_LEN 64

#define STOMP_FRAME_COMMAND_LEN 12
#define STOMP_FRAME_PAYLOAD_LEN 128

struct stomp_header {
    struct stomp_header *next;
    char name[STOMP_HEADER_NAME_LEN];
    char value[STOMP_HEADER_VALUE_LEN];
};

struct stomp_frame {
    struct stomp_header *headers;
    char command[STOMP_FRAME_COMMAND_LEN];
    char payload[STOMP_FRAME_PAYLOAD_LEN];
};

/* Usuwa naglowek ramki, wraz z zaleznoscmia */
void
stomp_frame_delete_header(struct stomp_header *header);

/* Usuwa cala ramke, wraz z zaleznoscmia */
void
stomp_frame_delete_frame(struct stomp_frame *frame);

/* Tworzy nowy obiekt naglowka, uzywany przez ramke */
struct stomp_header*
stomp_frame_new_header(const char *name, const char *value);

struct stomp_header*
stomp_frame_add_header(const char *name, const char *value, struct stomp_header *headers);

/* Tworzy nowy obiekt ramki */
struct stomp_frame*
stomp_frame_new_frame(const char *command, struct stomp_header *headers, const char *payload);

/* Szuka naglowka */		
struct stomp_header*
stomp_frame_find_header(const char *name, struct stomp_frame *frame);

/* Importuje ramke ze strumienia znakow */
struct stomp_frame*
stomp_frame_import(const char *stream, struct stomp_frame *frame);

/* Eksportuje ramke do strumienia znakow */
char*
stomp_frame_export(struct stomp_frame *frame);

/* Wyswietla zawartosc ramki */
void
stomp_frame_print(struct stomp_frame *frame);

/* Wyswietla ramke w surowej postaci */
void
stomp_frame_raw(struct stomp_frame *frame);

/* Podaje dlugosc ramki */
int
stomp_frame_length(struct stomp_frame *frame);

#endif /* FRAME_H_ */
