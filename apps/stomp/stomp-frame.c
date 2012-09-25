#include "stomp-frame.h"

#include "stomp-tools.h"
#include "stomp-memguard.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * frame.c
 *
 *  Created on: 23-03-2012
 *      Author: paoolo
 */

/* Usuwa naglowek ramki, wraz z zaleznosciami */
void _frame_delete_header(frame_header_t *header) {
	/* Brak naglowka - brak roboty */
	if(header == NULL) {
		return;
	}

	/* Usuwam kolejny naglowek */
	_frame_delete_header(header->next);

	/* Usuwam ciagi znakow */
	_del_ref(header->name);
	_del_ref(header->value);
}

/* Usuwa ramke, wraz z zaleznosciami */
void _frame_delete_frame(frame_t *frame) {

	/* Brak ramki - brak roboty */
	if(frame == NULL) {
		return;
	}

	/* Usuwamy naglowki */
	_frame_delete_header(frame->headers);

	/* Usuwamy wszystko po kolei */
	_del_ref(frame->command);
	_del_ref(frame->payload);
}

/* Tworzy pusty naglowek */
frame_header_t* __frame_new_header_empty() {

	/* Utworzenie pustego naglowka */
	frame_header_t *header = (frame_header_t*) _deref(_new_ref(sizeof(frame_header_t)));

	return header;
}

/* Tworzy naglowek o podanej nazwie i wartosci */
frame_header_t* _frame_new_header(char *name, char *value) {

	/* Utworzenie pustego naglowka */
	frame_header_t *header = __frame_new_header_empty();

	/* Uzupelnienie naglowka */
	header->name = _tools_strcpy(name);
	header->value = _tools_strcpy(value);

	/* Zwrocenie struktury */
	return header;
}
/* Tworzy pusta ramke */
frame_t* __frame_new_frame_empty() {
	/* Utworzenie pustej ramki */
	frame_t *frame = (frame_t*) _deref(_new_ref(sizeof(frame_t)));

	return frame;
}

/* Tworzy ramke o podanym poleceniu, liscie naglowkow i komunikacie do przeslania */
frame_t* _frame_new_frame(char *command, frame_header_t *headers, char *payload) {

	/* Utworzenie pustej ramki */
	frame_t *frame = __frame_new_frame_empty();

	/* Ustawienie wartosci typu ramki */
	frame->command = _tools_strcpy(command);
	/* Wstawienie listy naglowkow do ramki */
	frame->headers = headers;
	/* Ustawienie wartosci payload */
	frame->payload = _tools_strcpy(payload);

	/* Zwrocenie struktury ramki */
	return frame;
}

/* Okresla wielkosci ramki do przeslania przez siec */
int __frame_size_of_frame(frame_t *frame) {

	/* Sumaryczny rozmiar calej struktury */
	int sum = 0;

	/* Wskaznik do kolejnego pola naglowka */
	frame_header_t *ptr_frame_header;

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

/* Parsowanie naglowka ramki, tj. nazwy polecenia */
frame_t* __frame_parse_command(char *line, frame_t *frame) {

	/* W przypadku, gdy brak danych, wychodzimy */
	if (line == NULL) {
		return frame;

	} else {
		/* W przypadku, gdy dane sa, ale brak struktury */
		if (frame == NULL) {
			frame = __frame_new_frame_empty();
		}
	}

	/* Przypisanie komendy */
	frame->command = (char*) _deref(_ref(line));

	return frame;
}

/* Pozwala na wyszukanie danego naglowka, z podaniem nazwy naglowka. W razie
 * nieobecnosci naglowka, zwraca null. */
frame_header_t* _frame_find_header(char *name, frame_t *frame) {
	/* Wskaznik do poszukiwanego naglowka */
	frame_header_t *found_frame_header = NULL;

	/* Wskaznik uzywany do przeszukiwania listy naglowkow */
	frame_header_t *prt_frame_header = frame->headers;
	
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
frame_header_t* __frame_append_header(frame_header_t *header, frame_header_t *append) {

	/* Naglowek zostanie dodany na poczatek */
	append->next = header;

	return append;
}

/* Parsowanie pol naglowka ramki */
frame_header_t* __frame_parse_header(char *line, frame_header_t *header) {

	/* W przypadku, gdy brak danych, wychodzimy */
	if (line == NULL) {
		return header;

	} else {
		/* W przypadku, gdy dane sa, ale brak struktury */
		if (header == NULL) {
			header = __frame_new_header_empty();
		}
	}

	/* Pobranie nazwy pola naglowka */
	header->name = _tools_strtok(&line, _FRAME_SEPERATOR);
	/* Pobranie wartosci pola naglowka */
	header->value = _tools_strtok(&line, _FRAME_SEPERATOR);

	return header;
}

/* Importuje ramke ze strumienia znakow */
frame_t* _frame_import_frame(char *stream, frame_t *frame) {
	/* Kolejne linie ramki */
	char *line;

	/* W przypadku, gdy brak danych, wychodzimy */
	if (stream == NULL) {
		return frame;

	} else {
		/* W przypadku, gdy dane sa, ale brak struktury */
		if (frame == NULL) {
			frame = __frame_new_frame_empty();
		}
	}

	/* Pobranie pierwszego tokenu */
	line = _tools_strtok(&stream, _FRAME_LF);

	/* Parsowanie pierwszej linii - polecenia ramki */
	frame->command = _tools_strcpy(line);

	_del_ref(line);

	/* Przejscie do parsowania naglowka */
	line = _tools_strtok(&stream, _FRAME_LF);

	/* Do konca sekcji pol naglowka (?) */
	while (line != NULL && strcmp(line, "") != 0) {
		/* Parsowanie naglowka, zapis w odwroconej kolejnosci */
		frame->headers =
				__frame_append_header(frame->headers,
						__frame_parse_header(line, NULL));

		/* Usuniecie */
		_del_ref(line);

		/* Pobieranie kolejnych */
		line = _tools_strtok(&stream, _FRAME_LF);
	}
	/* Wedlug specyfikacji to powinno byc teraz LF pobrane, czyli nastepne
	 * pobranie to juz payload, a to po prostu usuwamy. */
	if(line != NULL) {
		_del_ref(line);

		/* Cala reszta to payload, az do 0x00, tj. '\0' */
		frame->payload = _tools_strcpy(stream);
	} else {
		/* Cos poszlo nie tak, ustawiam payload na pusty ciag znakow */
		frame->payload = (char*) _deref(_new_ref(sizeof(char)));
		*(frame->payload) = '\0';
	}

	return frame;
}

/* Eksportuje ramke do strumienia znakow */
char* _frame_export_frame(frame_t *frame) {
	/* Bufor danych do wyslania */
	char *stream = NULL;
	/* Wielkosc bufora */
	int size = 0;
	/* Przesuniecie w buforze, zmienna tymczasowa */
	int offset = 0, tmp = 0;
	/* Wskaznik do pola naglowka */
	frame_header_t *ptr_frame_header = NULL;

	/* Gdy brak ramki, koncze */
	if(frame == NULL) {
		return stream;
	}

	/* Okreslenie wielkosci bufora potrzebnego na przechowanie ramki */
	size = __frame_size_of_frame(frame);

	/* Utworzenie bufora */
	stream = (char*) _deref(_new_ref(sizeof(char) * size));

	/* Kopiowanie nazwy polecenia ramki */
	tmp = strlen(frame->command);
	strncpy(stream, frame->command, tmp);
	offset = offset + tmp;

	/* Dodanie znaku konca linii, po komendzie */
	stream[offset] = '\n';
	offset = offset + 1;

	/* Kopiowanie kolejnych pol naglowka */
	ptr_frame_header = frame->headers;
	while(ptr_frame_header != NULL) {
		/* Kopiowanie nazwy naglowka */
		tmp = strlen(ptr_frame_header->name);
		strncpy(stream+offset, ptr_frame_header->name, tmp);
		offset = offset + tmp;

		/* Dodanie separatora naglowka */
		stream[offset] = ':';
		offset = offset + 1;

		/* Kopiowanie wartosci przypisanej do naglowka */
		tmp = strlen(ptr_frame_header->value);
		strncpy(stream+offset, ptr_frame_header->value, tmp);
		offset = offset + tmp;

		/* Dodanie znaku konca linii, po naglowku */
		stream[offset] = '\n';
		offset = offset + 1;

		/* Przejscie do nastepnego naglowka, o ile jest */
		ptr_frame_header = ptr_frame_header->next;
	}

	/* Dodanie znaku konca linii, po wszystkich naglowkach,
	 * wg specyfikacji STOMP 1.1 */
	stream[offset] = '\n';
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

/* Drukuje w postaci opisowej zawartosc ramki.
 * @Deprecated */
void _frame_print_frame(frame_t *frame) {
	/* Kolejne pola naglowka */
	frame_header_t *header_frame_header;

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

/* Drukuje w postaci surowej zawartosc ramki, taka wartosc, jaka
 * zwroci nam _frame_export_frame(). @Deprecated */
void _frame_print_raw_frame(frame_t *frame) {
	/* Bufor ramki */
	char *stream = NULL;

	/* Gdy brak ramki, koncze */
	if(frame == NULL) {
		fprintf(stderr, "_error_ no __frame_frame_t\n");
		return;
	}
	stream = _frame_export_frame(frame);
	fprintf(stdout, "---START---\n%s^@\n---STOP---\n", stream);

	_del_ref(stream);
}
