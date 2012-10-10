#include "stomp-frame.h"

#include "stomp-tools.h"
#include "stomp-memguard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STOMP_NULL 0x00
#define STOMP_COLON 0x3a
#define STOMP_NEW_LINE 0x0a

void
stomp_frame_delete_header(struct stomp_header *header)
{
    if(header == NULL) {
        return;
    }

    stomp_frame_delete_header(header->next);

    stomp_del_ref(header->name);
    stomp_del_ref(header->value);
}

void
stomp_frame_delete_frame(struct stomp_frame *frame)
{
    if(frame == NULL) {
        return;
    }

    stomp_frame_delete_header(frame->headers);

    stomp_del_ref(frame->command);
    stomp_del_ref(frame->payload);
}

static struct stomp_header*
__frame_new_header_empty()
{
    struct stomp_header *header;
    header = (struct stomp_header*) stomp_deref(stomp_new_ref(sizeof(struct stomp_header)));
    return header;
}

struct stomp_header*
stomp_frame_new_header(const char *name, const char *value)
{
    struct stomp_header *header = __frame_new_header_empty();

    header->name = stomp_tools_strcpy(name);
    header->value = stomp_tools_strcpy(value);

    return header;
}

struct stomp_header*
stomp_frame_add_header(const char *name, const char *value, struct stomp_header *headers)
{
    struct stomp_header *header = stomp_frame_new_header(name, value);
    header->next = headers;
    return header;
}

static struct stomp_frame*
__frame_new_frame_empty()
{
    struct stomp_frame *frame;
    frame = (struct stomp_frame*) stomp_deref(stomp_new_ref(sizeof(struct stomp_frame)));
    return frame;
}

struct stomp_frame*
stomp_frame_new_frame(const char *command, struct stomp_header *headers, const char *payload)
{
    struct stomp_frame *frame = __frame_new_frame_empty();

    frame->command = stomp_tools_strcpy(command);
    frame->headers = headers;
    frame->payload = stomp_tools_strcpy(payload);

    return frame;
}

/* Parsowanie naglowka ramki, tj. nazwy polecenia */
static struct stomp_frame*
__frame_parse_command(const char *line, struct stomp_frame *frame)
{
    if (line == NULL) {
        /* W przypadku, gdy brak danych, wychodzimy */
        return frame;

    } else {
        /* W przypadku, gdy dane sa, ale brak struktury */
        if (frame == NULL) {
            frame = __frame_new_frame_empty();
        }
    }

    /* Przypisanie komendy */
    frame->command = (char*) stomp_deref(stomp_ref(line));

    return frame;
}

/* Pozwala na wyszukanie danego naglowka, z podaniem nazwy naglowka. W razie
 * nieobecnosci naglowka, zwraca null. */
struct stomp_header*
stomp_frame_find_header(const char *name, struct stomp_frame *frame)
{
    /* Wskaznik do poszukiwanego naglowka */
    struct stomp_header *found_frame_header = NULL;

    /* Wskaznik uzywany do przeszukiwania listy naglowkow */
    struct stomp_header *prt_frame_header = frame->headers;

    /* Szukamy po wszystkich naglowkach do momentu, gdy nie znajdziemy */
    while (prt_frame_header != NULL && found_frame_header == NULL) {
        if (strcmp(prt_frame_header->name, name) == 0) {
            /* Jesli sie nazwa poszukiwanego naglowka zgadza sie z nazwa naglowka */
            found_frame_header = prt_frame_header;
        } else {
            /* Przechodzimy do kolejnego naglowka */
            prt_frame_header = prt_frame_header->next;
        }
    }

    /* Zwracamy znaleziony naglowek lub NULL, gdy nie ma */
    return found_frame_header;
}

/* Dodanie naglowka do listy naglowkow */
static struct stomp_header*
__frame_append_header(struct stomp_header *header, struct stomp_header *append)
{
    /* Naglowek zostanie dodany na poczatek */
    append->next = header;

    return append;
}

/* Parsowanie pol naglowka ramki */
static struct stomp_header*
__frame_parse_header(const char *line, struct stomp_header *header)
{
    if (line == NULL) {
        /* W przypadku, gdy brak danych, wychodzimy */
        return header;

    } else {
        /* W przypadku, gdy dane sa, ale brak struktury */
        if (header == NULL) {
            header = __frame_new_header_empty();
        }
    }

    /* Pobranie nazwy pola naglowka */
    header->name = stomp_tools_strtok(&line, STOMP_COLON);
    /* Pobranie wartosci pola naglowka */
    header->value = stomp_tools_strtok(&line, STOMP_COLON);

    return header;
}

/* Importuje ramke ze strumienia znakow */
struct stomp_frame*
stomp_frame_import(const char *stream, struct stomp_frame *frame)
{
    /* Kolejne linie ramki */
    char *line;

    if (stream == NULL) {
        /* W przypadku, gdy brak danych, wychodzimy */
        return frame;

    } else {
        /* W przypadku, gdy dane sa, ale brak struktury */
        if (frame == NULL) {
            frame = __frame_new_frame_empty();
        }
    }

    /* Pobranie pierwszego tokenu */
    line = stomp_tools_strtok(&stream, STOMP_NEW_LINE);

    /* Parsowanie pierwszej linii - polecenia ramki */
    frame->command = stomp_tools_strcpy(line);

    stomp_del_ref(line);

    /* Przejscie do parsowania naglowka */
    line = stomp_tools_strtok(&stream, STOMP_NEW_LINE);

    /* Do konca sekcji pol naglowka (?) */
    while (line != NULL && strcmp(line, "") != 0) {
        /* Parsowanie naglowka, zapis w odwroconej kolejnosci */
        frame->headers = __frame_append_header(frame->headers, __frame_parse_header(line, NULL));

        /* Usuniecie */
        stomp_del_ref(line);

        /* Pobieranie kolejnych */
        line = stomp_tools_strtok(&stream, STOMP_NEW_LINE);
    }

    /* Wedlug specyfikacji to powinno byc teraz LF pobrane, czyli nastepne
     * pobranie to juz payload, a to po prostu usuwamy. */
    if(line != NULL) {
        stomp_del_ref(line);
        
        /* Cala reszta to payload, az do 0x00, tj. '\0' */
        frame->payload = stomp_tools_strcpy(stream);
        
    } else {
        /* Cos poszlo nie tak, ustawiam payload na pusty ciag znakow */
        frame->payload = (char*) stomp_deref(stomp_new_ref(sizeof(char)));
        *(frame->payload) = 0x00;
    }

    return frame;
}

/* Eksportuje ramke do strumienia znakow */
char*
stomp_frame_export(struct stomp_frame *frame) {
    /* Bufor danych do wyslania */
    char *stream = NULL;
    /* Wielkosc bufora */
    int size = 0;
    /* Przesuniecie w buforze, zmienna tymczasowa */
    int offset = 0, tmp = 0;
    /* Wskaznik do pola naglowka */
    struct stomp_header *ptr_frame_header = NULL;

    /* Gdy brak ramki, koncze */
    if(frame == NULL) {
        return stream;
    }

    /* Okreslenie wielkosci bufora potrzebnego na przechowanie ramki */
    size = stomp_frame_length(frame);

    /* Utworzenie bufora */
    stream = (char*) stomp_deref(stomp_new_ref(sizeof(char) * size));

    /* Kopiowanie nazwy polecenia ramki */
    tmp = strlen(frame->command);
    strncpy(stream, frame->command, tmp);
    offset = offset + tmp;

    /* Dodanie znaku konca linii, po komendzie */
    stream[offset] = STOMP_NEW_LINE;
    offset = offset + 1;

    /* Kopiowanie kolejnych pol naglowka */
    ptr_frame_header = frame->headers;
    while(ptr_frame_header != NULL) {
        /* Kopiowanie nazwy naglowka */
        tmp = strlen(ptr_frame_header->name);
        strncpy(stream+offset, ptr_frame_header->name, tmp);
        offset = offset + tmp;

        /* Dodanie separatora naglowka */
        stream[offset] = STOMP_COLON;
        offset = offset + 1;

        /* Kopiowanie wartosci przypisanej do naglowka */
        tmp = strlen(ptr_frame_header->value);
        strncpy(stream+offset, ptr_frame_header->value, tmp);
        offset = offset + tmp;

        /* Dodanie znaku konca linii, po naglowku */
        stream[offset] = STOMP_NEW_LINE;
        offset = offset + 1;

        /* Przejscie do nastepnego naglowka, o ile jest */
        ptr_frame_header = ptr_frame_header->next;
    }

    /* Dodanie znaku konca linii, po wszystkich naglowkach,
     * wg specyfikacji STOMP 1.1 */
    stream[offset] = STOMP_NEW_LINE;
    offset = offset + 1;

    /* Dodanie calej zawartosci payload'u ramki */
    if(frame->payload != NULL) {
        tmp = strlen(frame->payload);
        strncpy(stream+offset, frame->payload, tmp);
        offset = offset + tmp;
    }

    /* Zakonczenie ramki znakiem NULL */
    stream[offset] = 0;
    offset = offset + 1;

    /* Powinno zajsc :) */
    if(offset != size) {
        printf("_assert_ %d != %d\n", offset, size);
        /* TODO obsluzyc sytuacje */
    }

    /* Zwracmy strumien do wyslania */
    return stream;
}

void
stomp_frame_print(struct stomp_frame *frame) {
    /* Kolejne pola naglowka */
    struct stomp_header *header_frame_header;

    /* Gdy brak ramki, koncze */
    if(frame == NULL) {
        fprintf(stderr, "_error_ no frame\n");
        return;
    }

    /* Wyswietlenie wartosci polecenia */
    fprintf(stdout, "command:\n# %s\n", frame->command);

    /* Wyswietlenie pol naglowka */
    fprintf(stdout, "headers:\n");
    header_frame_header = frame->headers;
    while(header_frame_header != NULL) {
        fprintf(stdout, "# %s:%s\n", header_frame_header->name, header_frame_header->value);

        header_frame_header = header_frame_header->next;
    }

    /* Wyswietlenie zawartosci ramki */
    if(frame->payload != NULL) {
        fprintf(stdout, "payload:\n# %s\n", frame->payload);
    }
}

void
stomp_frame_raw(struct stomp_frame *frame) {
    /* Bufor ramki */
    char *stream = NULL;

    /* Gdy brak ramki, koncze */
    if(frame == NULL) {
        fprintf(stderr, "_error_ no __frame_frame_t\n");
        return;
    }
    stream = stomp_frame_export(frame);
    fprintf(stdout, "---START---\n%s^@\n---STOP---\n", stream);

    stomp_del_ref(stream);
}

int
stomp_frame_length(struct stomp_frame *frame)
{
    int sum = 0;

    /* Wskaznik do kolejnego pola naglowka */
    struct stomp_header *ptr_frame_header;

    /* Pusta, no to zero wysylam */
    if(frame == NULL) {
        return sum;
    }

    /* Dodanie wartosci dlugosci polecenia */
    sum = sum + strlen(frame->command) + 1; /* +1 bo na '\n' */

    /* Dodawanie wartosci dlugosci pol naglowka */
    ptr_frame_header = frame->headers;

    /* Odpytywanie kolejnych pol naglowkow, az do konca */
    while(ptr_frame_header != NULL) {
        /* Pobranie dlugosci naglowa */
        sum = sum + strlen(ptr_frame_header->name)
                + strlen(ptr_frame_header->value) + 2; /* +2 bo na ':' i '\n' */

        /* Przechodzimy do nastepnego naglowka, o ile jest */
        ptr_frame_header = ptr_frame_header->next;
    }

    /* Dodanie wartosci wielkosci payloadu */
    if(frame->payload != NULL) {
        sum = sum + strlen(frame->payload);
    }

    sum = sum + 2; /* +2 bo na '\n' po naglowku i '\0' na koniec ramki */

    return sum;
}