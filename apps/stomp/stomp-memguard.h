#include <stdlib.h>

/*
 * memguard.h
 *
 *  Created on: 01-04-2012
 *      Author: paoolo
 */

#ifndef FACTORY_H_
#define FACTORY_H_

/* Struktura przechowujaca informacje o wskazywanym obiekcie. Brak tylko informacji
 * o tym kto trzyma te referencje. */
struct _reference_t {
	/* Czy uzywana? */
	char used;
	/* Wskaznik do faktycznego obiektu */
	void *handle;
	/* Informacja o ilosci wystapien, tzw. licznik referencji */
	int counter;
	/* Wielkosc obiektu, trzymanego pod wskaznikiem */
	ssize_t size;
};
typedef struct _reference_t reference_t;

#define _MEMGUARD_DISABLE 0
#define _MEMGUARD_ENABLE 1

/* Czy mechanizm straznika pamieci jest aktywny? */
extern int _memguard_is_active;

/* Wielkosc tablicy wskaznikow do referencji */
extern int _memguard_ref_array_len;

/* Obecnie uzywana ilosc wskaznikow */
extern int _memguard_ref_usage;

/* Tablica wszystkich referencji */
extern reference_t **_memguard_ref_array;

/* Inicjalizacja modulu zarzadzania pamiecia, powinno byc zawolane
 * na poczatku programu */
void _memguard_init();

/* Zezwolenie na uzywanie memguard'a, domyslnie po inicjalizacji, memguard
 * jest w stanie enabled */
void _memguard_enable();

/* Tymczasowe wylaczenie memguarda'a, ale nie wyzerowanie informacji */
void _memguard_disable();

/* Zatrzymanie straznika pamieci i usuniecie informacji przechowywanych
 * przez niego */
void _memguard_flush();

/* Zwraca wskaznik do obiektu, ktory jest trzymany przez referencje */
void* _deref(reference_t *__ref_reference_t);

/* Operacja odwrotna do dereferencji */
reference_t* _ref(void *__ptr);

/* Dodaje nowa referencje */
reference_t* _new_ref(ssize_t __size);

/* Usuniecie referencji do obiektu */
void _del_ref(void *__ptr);

#endif /* FACTORY_H_ */
