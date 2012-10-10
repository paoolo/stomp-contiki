#include "stomp-memguard.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * memguard.c
 *
 * Modul dostarcza mechanizmow tworzenia i zwalniania pamieci dla struktur. Nie zastepuje on
 * mechanizmu alokacji pamieci i jej zwalniania, tj. nadal operujemy na faktycznych wskaznikach
 * do pamieci, przy czym dysponujemy dodatkowo mechanizmami detekcji zajetosci pamieci oraz jej
 * uzycia. Gdy prawidlowo bedzie sie korzystalo z tego mechanizmu, to teoretycznie nie dojdzie
 * do wyciekow pamieci, czy porzucania nieuzywanych obiektow pamieci.
 *
 * Byc moze kiedys zastapimy mechanizm alokacji pamieci i dostarczymy wlasny mechanizm dostepu do
 * pamieci, poprzez odpowiednie funkcje. Poki co modul jest dodatkiem, a nie warstwa posredniczaca
 * w dostepie do pamieci.
 *
 *  Created on: 01-04-2012
 *      Author: paoolo
 */

/* O ile bedzie powieksza tablica przechowujaca wskazniki do referencji */
#define STOMP_FACTORY_ADD_REF 8

/* Czy mechanizm straznika pamieci jest aktywny? */
int stomp_memguard_is_active;

/* Wielkosc tablicy wskaznikow do referencji */
int stomp_memguard_ref_array_len;

/* Obecnie uzywana ilosc wskaznikow */
int stomp_memguard_ref_usage;

/* Tablica wszystkich referencji */
struct stomp_reference **stomp_memguard_ref_array;

/* Inicjalizacja modulu zarzadzania pamiecia */
void stomp_memguard_init() {
    /* Ustawienie flagi dzialania mechanizmu na enable */
    stomp_memguard_enable();

    /* Ustawienie pozostalych parametrow */
    stomp_memguard_flush();
}

/* Uaktywnia mechanizm straznika pamieci. Wszelkie operacje zwiazane
 * z alokacja pamieci i jej zwalnianiem sa monitorowane przez
 * straznika pamieci */
void stomp_memguard_enable() {
    stomp_memguard_is_active = STOMP_MEMGUARD_ENABLE;
}

/* Zatrzymuje mechanizm straznika pamieci. Operacje alokacji
 * i zwalniania pamieci nie sa kontrolowane. Wymagana jest kontrola
 * wlasna. */
void stomp_memguard_disable() {
    stomp_memguard_is_active = STOMP_MEMGUARD_DISABLE;
}

/* Zeruje informacje przechowywane przez straznika pamieci. */
void stomp_memguard_flush() {
    /* Zmienna pomocnicza */
    int i;

    /* Usuwamy poprzednia tablice, o ile jest */
    if(stomp_memguard_ref_array != NULL) {
        /* Przejscie po wszystkich wskaznikach do referencji */
        for(i = 0; i < stomp_memguard_ref_array_len; i++) {
            free(stomp_memguard_ref_array[i]);
        }
        /* Usuniecie tablicy wskaznikow */
        free(stomp_memguard_ref_array);
    }

    /* Ustawienie poczatkowej ilosci wskaznikow */
    stomp_memguard_ref_array_len = STOMP_FACTORY_ADD_REF;
    /* Ustawienie uzywanej ilosci wskaznikow */
    stomp_memguard_ref_usage = 0;
    /* Inicjalizacja tablicy wskaznikow do referencji */
    stomp_memguard_ref_array = (struct stomp_reference**) malloc(sizeof(struct stomp_reference*) * stomp_memguard_ref_array_len);

    /* To nie powinno nigdy zajsc, ale jak zajdzie, to mamy problem.
     * Konczymy tym samy proces. */
    if(stomp_memguard_ref_array == NULL) {
        printf("_panic_ cannot alocate memory for reference array\n");
        exit(-1);
    }

    /* Ustawienie wszystkich referencji na NULL, bowiem na nic nie wskazuja */
    memset(stomp_memguard_ref_array, 0, sizeof(struct stomp_reference*) * stomp_memguard_ref_array_len);
}

/* Powiekszenie tablicy o wstepnie ustalona ilosc kolejnych wskaznikow */
static void __memguard_enlarge() {
    /* Zmienna pomocnicza */
    int i = 0;

    /* Wskaznik do nowej tablicy wskaznikow do referencji */
    struct stomp_reference **tmp_ref_array;

    /* Zwiekszenie tablicy o _FACTORY_ADD_REF nowych wskaznikow do referencji */
    stomp_memguard_ref_array_len = stomp_memguard_ref_array_len + STOMP_FACTORY_ADD_REF;

    /* Utworzenie wiekszej tablicy */
    tmp_ref_array = (struct stomp_reference**) malloc(sizeof(struct stomp_reference*) * stomp_memguard_ref_array_len);

    /* To nie powinno nigdy zajsc, ale jak zajdzie, to mamy problem.
     * Konczymy tym samy proces. */
    if(tmp_ref_array == NULL) {
        printf("Cannot alocate memory for reference array");
        exit(-1);
    }

    /* Ustawienie wszystkich referenji na NULL, bowiem na nic nie wskazuja */
    memset(tmp_ref_array, 0, sizeof(struct stomp_reference*) * stomp_memguard_ref_array_len);

    /* Kopiowanie referencji z obecnej do nowej tablicy */
    for(i = 0; i < stomp_memguard_ref_array_len - STOMP_FACTORY_ADD_REF; i++) {
        /* Kopiowanie */
        tmp_ref_array[i] = stomp_memguard_ref_array[i];
    }

    /* Usuniecie poprzedniej tablicy wskaznikow, ale nie obiektow, na ktore wskazuja */
    free(stomp_memguard_ref_array);

    /* Zapisanie nowego wskaznika */
    stomp_memguard_ref_array = tmp_ref_array;
}

/* Zmniejsza tablice, jednoczesnie defragmentujac ja, poprzez usuwanie pustych miejsc */
static void __memguard_shrink() {
    /* Zmienne pomocnicze */
    int i = 0, j = 0;

    /* Wskaznik do nowej tablicy wskaznikow referencji, ktorego
     * wielkosc jest zgodna z iloscia obecnie uzywanych obiektow
     * referencji. */
    struct stomp_reference **tmp_ref_array = (struct stomp_reference**) malloc(sizeof(struct stomp_reference*) * stomp_memguard_ref_usage);

    /* To nie powinno nigdy zajsc, ale jak zajdzie, to mamy problem.
     * Konczymy tym samy proces. */
    if(tmp_ref_array == NULL) {
        printf("Cannot alocate memory for reference array\n");
        exit(-1);
    }

    /* Kopiowanie uzywanych obiektow referencji */
    for(i=0, j=0; i < stomp_memguard_ref_array_len && j < stomp_memguard_ref_usage; i++) {
        /* Jesli tylko cos tam w starej tablicy jest to .. */
        if(stomp_memguard_ref_array[i] != NULL) {
            /* .. kopiujemy */
            tmp_ref_array[j] = stomp_memguard_ref_array[i];
            j = j+1;
        }
    }

    /* Usuwanie poprzedniej tablicy referencji, bez usuwania obiektow */
    free(stomp_memguard_ref_array);

    /* Zapisanie nowej wartosci wielkosci tablicy, rownej ilosci uzywanych
     * obiektow referencji w starej tablicy */
    stomp_memguard_ref_array_len = stomp_memguard_ref_usage;
    /* Zapisanie nowego wskazniki */
    stomp_memguard_ref_array = tmp_ref_array;
}

/* Zwraca wskaznik do obiektu, ktory jest trzymany przez referencje */
void* stomp_deref(struct stomp_reference *ref) {
    /* Sprawdza, czy mamy cos do dereferencji */
    if(ref != NULL) {
        /* Sprawdzamy czy straznik jest aktywny */
        if(stomp_memguard_is_active) {
            /* Powiekszenie ilosci uzycia tego wskaznika o jeden */
            ref->counter = ref->counter + 1;
        }

        /* Dokonujemy faktycznej dereferencji */
        return ref->handle;

    } else {
        /* Sygnalizujemy wystapienie bledu wew. funkji, w tym wypadku
         * jest to taki NullPointerException */
        return NULL;
    }
}

/* Dodanie referencji do listy wszystkich referencji. Jesli brak miejsca,
 * to automatycznie zostanie powiekszona tablica referencji. */
static void __add_ref(struct stomp_reference *ref) {
    /* Zmienna pomocnicza */
    int i = 0;

    /* Jesli nie mamy co dodawac, to po co sie trudzic */
    if(ref == NULL) {
        return;
    }

    /* Jesli ilosc zajmowany wskaznikow referencji jest rowna ilosci
     * dostepnych wskaznikow w tablicy, to powiekszamy tablice. */
    if(stomp_memguard_ref_usage >= stomp_memguard_ref_array_len) {
        __memguard_enlarge();
    }

    /* Szukamy wolnego miejsca w tablicy wskaznikow referencji */
    for(i = 0; i < stomp_memguard_ref_array_len && stomp_memguard_ref_array[i] != NULL; i++) {
        /* do nothing :) */
    }

    /* Zapisanie wskaznika w puste miejsce */
    if(i < stomp_memguard_ref_array_len && stomp_memguard_ref_array[i] == NULL) {
        stomp_memguard_ref_array[i] = ref;
        stomp_memguard_ref_usage = stomp_memguard_ref_usage + 1;
    } else {
        printf("Cannot save reference\n");
        /* TODO obsluzyc sytuacja niemozliwosci zapisania referencji */
    }
}

/* Operacja odwrotna, uzyskujemy wskaznik do miejsca w tablicy */
static struct stomp_reference** __ptr_ref(void *ptr) {
    /* Zmienna pomocnicza */
    int i = 0;
    /* Wskaznik do obiektu zwracacanego */
    struct stomp_reference **ret = NULL;

    /* Przeszukiwanie liniowe tablicy, celem znalezienia referencji */
    for(i=0; i < stomp_memguard_ref_array_len && ret == NULL; i++) {
        /* Moze sie zdarzyc dziura */
        if(stomp_memguard_ref_array[i] != NULL) {
            /* Jesli obiekt referencji wskazuje na podany wskaznik
             * do pamieci, to wychodzimy z nim. */
            if ((stomp_memguard_ref_array[i])->handle == ptr) {
                /* Uzyskanie wskaznika do wskaznika referencji */
                ret = &(stomp_memguard_ref_array[i]);
            }
        }
    }

    /* Jesli okaze sie, ze nie ma obiektu referencji, to znaczy, ze
     * mamy do czynienia z jakas anomalia lub zaniedbaniem. Zas nie
     * mozemy dodac referencji, bowiem brak nam informacji o wielkosci
     * tej struktury, a wskaznik mamy do void. Dlatego tez zwracany jest
     * void, ktory mowi to co NoReferenceException. */
    return ret;
}

struct stomp_reference* stomp_ref(void *ptr) {
    /* Zmienna pomocnicza */
    struct stomp_reference **ret = __ptr_ref(ptr);

    /* Zwracamy referencje do wskaznika, o ile istnieje */
    if(ret != NULL) {
        return *ret;
    } else {
        return NULL;
    }
}

/* Dodaje nowo utworzony obiekt do zbioru wszystkich obiektów, wymagane podanie
 * wielkosc wskazywanego obiektu. Referencja jest automatycznie dodawana do
 * tablicy wszystkich referencji. */
static struct stomp_reference* __new_ref(void *obj, ssize_t size) {
    /* Przechowuje obiekt nowej referencji, na podstawie wskaznika do obiektu */
    struct stomp_reference *ret = NULL;

    /* Jesli przekazujemy wskaznik do null'a, to zaprzestajamy wykonywanie
     * dalszych kroków. */
    if(obj == NULL) {
        return NULL;
    }

    /* Utworzenie obiektu referencji do obiektu */
    ret = (struct stomp_reference*) malloc(sizeof(struct stomp_reference));

    /* Jesli nie nastapilo utworzenie obiektu refenecji */
    if(ret == NULL) {
        return NULL;
    }

    /* Zerowanie zawartosc */
    memset(ret, 0, sizeof(struct stomp_reference));

    /* Ustawienie obiektu referencji */
    ret->handle = obj;
    /* Ustawienie licznika na wartosc 1 */
    ret->counter = 1;
    /* Ustawienie wielkosci obiektu */
    ret->size = size;

    /* Sprawdzamy, czy straznik pamieci jest aktywny */
    if(stomp_memguard_is_active) {
        /* Dodanie referencji do listy wszystkich dostepnych referencji. */
        __add_ref(ret);
    }

    return ret;
}

/* Tworzy nowy obiekt, o zadanej wielkosci i tworzy obiekt referencji dla tego obiektu. */
struct stomp_reference* stomp_new_ref(ssize_t size) {
    /* Obiekt referencji, ktory zostanie zwrocony */
    struct stomp_reference *ref;
    /* Utworzenie obiektu z podaniem okreslonej wielkosci */
    void *obj = (void*) malloc(size);

    /* Zerowanie zawartosci */
    memset(obj, 0, size);

    /* Moze nie zostac utworzony obiekt o podanej wielkosci */
    if(obj == NULL) {
        /* TODO obsluzyc sytuacje niemozliwosci alokacji pamieci */
        printf("Cannot alocate memory\n");
        return NULL;
    }

    /* Utworzenie obiektu referencji */
    ref = __new_ref(obj, size);

    return ref;
}

static void __del_ref(struct stomp_reference **ref) {
    /* Usuwamy to na co wskazywal przekazany wskaznik */
    free((*ref)->handle);

    /* Usuwamy obiekt referencji i zwalniamy miejsce w tablicy */
    free(*ref);

    /* Ustawienie na NULL */
    *ref = NULL;
}

/* Usuniecie referencji do obiektu, powoduje tylko zmniejszenie wskaznika
 * ilosci referencji o jeden. Gdy dojdzie do zera, nastepuje faktyczne
 * usuniecie obiektu z pamieci i tablicy referencji obiektow */
void stomp_del_ref(void *ptr) {
    /* Wskaznik do referencji */
    struct stomp_reference **ref = NULL;

    /* Jak nie mamy referencji, to co usuwac? */
    if(ptr == NULL) {
        return;
    }

    /* Szukamy referencji do podanego wskaznika */
    ref = __ptr_ref(ptr);

    /* Brak referencji? */
    if(ref == NULL || *ref == NULL) {
        /* Nie wiem co zrobic, nie moge usunac, bo nie wiem co
         * to za obiekt */
        return;
    }

    /* Sprawdzam czy straznik jest aktywny */
    if(stomp_memguard_is_active) {
        /* Pomniejsze wskaznika ilosci referencji o jeden */
        (*ref)->counter = (*ref)->counter - 1;

        /* Gdy osiagnie zero, to usuwamy */
        if((*ref)->counter == 0) {
            /* Usuniecie referencji */
            __del_ref(ref);

            /* Pomniejszamy ilosc uzytych wskaznikow o jeden */
            stomp_memguard_ref_usage = stomp_memguard_ref_usage - 1;
        }
    } else {
        /* Usuniecie referencji */
        __del_ref(ref);
    }
}
