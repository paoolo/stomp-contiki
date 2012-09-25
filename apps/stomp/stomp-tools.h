/*
 * tools.h
 *
 *  Created on: 24-03-2012
 *      Author: paoolo
 */

#ifndef TOOLS_H_
#define TOOLS_H_

/* Zmodyfikowane kopiowanie stringa (z tworzeniem nowego string'a) */
char* _tools_strcpy(char *str);

/* Zmodyfikowane laczenie dwoch stringow (z tworzeniem nowego string'a) */
char* _tools_strcat(char *dst, char *src);

/* Zmodyfikowane wydzielanie kolejnych elementow string'a oddzielonych delim'em.
 * Modyfikuje przekazywany parametr str, przesuwa o ilosc odciety znakow, zwracanych
 * przez funkcje. */
char* _tools_strtok(char **str, char delim);

#endif /* TOOLS_H_ */
