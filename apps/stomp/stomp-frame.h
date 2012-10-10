#ifndef FRAME_H_
#define FRAME_H_

struct stomp_header {
    struct stomp_header *next;
    char *name;
    char *value;
};

struct stomp_frame {
    char *command;
    struct stomp_header *headers;
    char *payload;
};

/* Usuwa naglowek ramki, wraz z zaleznoscmia */
void
stomp_frame_delete_header(struct stomp_header *header);

/* Usuwa cala ramke, wraz z zaleznoscmia */
void
stomp_frame_delete_frame(struct stomp_frame *frame);

/* Tworzy nowy obiekt naglowka, uzywany przez ramke */
struct stomp_header*
stomp_frame_new_header(char *name, char *value);

struct stomp_header*
stomp_frame_add_header(char *name, char *value, struct stomp_header *headers);

/* Tworzy nowy obiekt ramki */
struct stomp_frame*
stomp_frame_new_frame(char *command, struct stomp_header *headers, char *payload);

/* Szuka naglowka */		
struct stomp_header*
stomp_frame_find_header(char *name, struct stomp_frame *frame);

/* Importuje ramke ze strumienia znakow */
struct stomp_frame*
stomp_frame_import(char *stream, struct stomp_frame *frame);

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
