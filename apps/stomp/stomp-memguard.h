#ifndef FACTORY_H_
#define FACTORY_H_

#include <stdlib.h>

#define STOMP_MEMGUARD_DISABLE 0
#define STOMP_MEMGUARD_ENABLE 1

/* Struktura przechowujaca informacje o wskazywanym obiekcie. Brak tylko informacji
 * o tym kto trzyma te referencje. */
struct stomp_reference {
    /* Czy uzywana? */
    char used;
    /* Wskaznik do faktycznego obiektu */
    void *handle;
    /* Informacja o ilosci wystapien, tzw. licznik referencji */
    int counter;
    /* Wielkosc obiektu, trzymanego pod wskaznikiem */
    ssize_t size;
};

/* Czy mechanizm straznika pamieci jest aktywny? */
extern int stomp_memguard_is_active;

/* Wielkosc tablicy wskaznikow do referencji */
extern int stomp_memguard_ref_array_len;

/* Obecnie uzywana ilosc wskaznikow */
extern int stomp_memguard_ref_usage;

/* Tablica wszystkich referencji */
extern struct stomp_reference **stomp_memguard_ref_array;

/* Inicjalizacja modulu zarzadzania pamiecia, powinno byc zawolane
 * na poczatku programu */
void stomp_memguard_init();

/* Zezwolenie na uzywanie memguard'a, domyslnie po inicjalizacji, memguard
 * jest w stanie enabled */
void stomp_memguard_enable();

/* Tymczasowe wylaczenie memguarda'a, ale nie wyzerowanie informacji */
void stomp_memguard_disable();

/* Zatrzymanie straznika pamieci i usuniecie informacji przechowywanych
 * przez niego */
void stomp_memguard_flush();

/* Zwraca wskaznik do obiektu, ktory jest trzymany przez referencje */
void* stomp_deref(struct stomp_reference *ref);

/* Operacja odwrotna do dereferencji */
struct stomp_reference* stomp_ref(void *ptr);

/* Dodaje nowa referencje */
struct stomp_reference* stomp_new_ref(ssize_t size);

/* Usuniecie referencji do obiektu */
void stomp_del_ref(void *__ptr);

#endif /* FACTORY_H_ */
