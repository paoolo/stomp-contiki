#ifndef TOOLS_H_
#define TOOLS_H_

#include <stdlib.h>

/* Malloc and memset new memory block */
void*
stomp_tools_new(int size);

/* Create new string and copy */
unsigned char*
stomp_tools_strcpy(const unsigned char *str);

/* Create new string and copy n chars */
unsigned char*
stomp_tools_strncpy(const unsigned char *src, int n);

/* Create new string and concat both */
unsigned char*
stomp_tools_strcat(const unsigned char *first, const unsigned char *second);

/* Create new string and copy substr */
void
stomp_tools_substr(const unsigned char *src, unsigned char *dest, int offset, int length);

/* Create new string and copy substr ended with delim */
int
stomp_tools_substr_to(const unsigned char *src, unsigned char *dest, int offset, unsigned char delim);

/* C Macros used to alloc and free memory */
#define NEW(T) (T*)stomp_tools_new(sizeof(T))
#define NEW_ARRAY(T, LEN) (T*)stomp_tools_new(sizeof(T) * (LEN))

#define DELETE(PTR) free(PTR)

unsigned char*
stomp_tools_long_to_bytes(const unsigned long value_long);

unsigned long
stomp_tools_bytes_to_long(const unsigned char *value_bytes);

#endif /* TOOLS_H_ */
