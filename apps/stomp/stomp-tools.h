/*
 * tools.h
 *
 *  Created on: 24-03-2012
 *      Author: paoolo
 */

#ifndef TOOLS_H_
#define TOOLS_H_

/* Create new string and copy */
char* stomp_tools_strcpy(char *str);

/* Create new string and concat both */
char* stomp_tools_strcat(char *dst, char *src);

/* Get next token splited by delim */
char* stomp_tools_strtok(char **str, char delim);

/* Copy N characters */
char* stomp_tools_strncpy(char *dest, const char *src, size_t n);

#endif /* TOOLS_H_ */
