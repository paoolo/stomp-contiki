#include "stomp-strings.h"

#ifndef FRAME_H_
#define FRAME_H_

#ifdef	__cplusplus
extern "C" {
#endif

    struct stomp_header {
        struct stomp_header *next;
        char *name;
        char *value;
    };
    typedef struct stomp_header stomp_header_t;

    struct stomp_frame {
        char *command;
        struct stomp_header *headers;
        char *payload;
    };
    typedef struct stomp_frame stomp_frame_t;

    /* Usuwa naglowek ramki, wraz z zaleznoscmia */
    void stomp_frame_delete_header(stomp_header_t *header);

    /* Usuwa cala ramke, wraz z zaleznoscmia */
    void stomp_frame_delete_frame(stomp_frame_t *frame);

    /* Tworzy nowy obiekt naglowka, uzywany przez ramke */
    stomp_header_t* stomp_frame_new_header(char *name, char *value);

    /* Tworzy nowy obiekt ramki */
    stomp_frame_t* stomp_frame_new_frame(char *command, stomp_header_t *headers, char *payload);

    /* Szuka naglowka */		
    stomp_header_t* stomp_frame_find_header(char *name, stomp_frame_t *frame);

    /* Importuje ramke ze strumienia znakow */
    stomp_frame_t* stomp_frame_import(char *stream, stomp_frame_t *frame);

    /* Eksportuje ramke do strumienia znakow */
    char* stomp_frame_export(stomp_frame_t *frame);

    /* Wyswietla zawartosc ramki */
    void stomp_frame_print(stomp_frame_t *frame);

    /* Wyswietla ramke w surowej postaci */
    void stomp_frame_raw(stomp_frame_t *frame);

    /* Podaje dlugosc ramki */
    int stomp_frame_length(stomp_frame_t *frame);

#ifdef	__cplusplus
}
#endif

/* Wielkosc ramki surowej na podstawie przekazanej struktury */
#endif /* FRAME_H_ */
