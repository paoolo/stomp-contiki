#ifndef TOOLS_H_
#define TOOLS_H_

#include <stdlib.h>

/* Malloc and memset new memory block */
void*
stomp_tools_new(int size);

/* Create new string and copy */
char*
stomp_tools_strcpy(const char *str);

/* Create new string and copy n chars */
char*
stomp_tools_strncpy(const char *src, int n);

/* Create new string and concat both */
char*
stomp_tools_strcat(const char *first, const char *second);

/* Create new string and copy substr */
void
stomp_tools_substr(const char *src, char *dest, int offset, int length);

/* Create new string and copy substr ended with delim */
int
stomp_tools_substr_to(const char *src, char *dest, int offset, char delim);

/* C Macros used to alloc and free memory */
#define NEW(T) (T*)stomp_tools_new(sizeof(T))
#define NEW_ARRAY(T, LEN) (T*)stomp_tools_new(sizeof(T) * (LEN))

#define DELETE(PTR) free(PTR); PTR = NULL

#endif /* TOOLS_H_ */
