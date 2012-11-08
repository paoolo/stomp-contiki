#include "stomp-tools.h"
#include "simple-stomp.h"

#include <string.h>
#include <stdlib.h>

void*
stomp_tools_new(int size) {
    void *ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}

unsigned char*
stomp_tools_strcpy(const unsigned char *src) {
    unsigned char *dest = NULL;
    int len = 0;

    len = strlen((const char*) src);

    if (src != NULL) {
        dest = NEW_ARRAY(unsigned char, len + 1);
        strcpy((char*) dest, (const char*) src);
    }

    return dest;
}

unsigned char*
stomp_tools_strncpy(const unsigned char *src, int n) {
    unsigned char *dest = NULL;
    int len = 0;

    len = strlen((const char*) src);

    if (src != NULL) {
        if (n > len) {
            dest = NEW_ARRAY(unsigned char, len + 1);
        } else {
            dest = NEW_ARRAY(unsigned char, n + 1);
        }
        strncpy((char*) dest, (const char*) src, n);
    }

    return dest;
}

unsigned char*
stomp_tools_strcat(const unsigned char *first, const unsigned char *second) {
    unsigned char *ret = NULL;

    if (first == NULL && second != NULL) {
        ret = stomp_tools_strcpy(second);

    } else if (first != NULL && second == NULL) {
        ret = stomp_tools_strcpy(first);

    } else if (first != NULL && second != NULL) {
        ret = NEW_ARRAY(unsigned char, strlen((const char*) first) + strlen((const char*) second) + 1);

        strcpy((char*) ret, (const char*) first);
        strcat((char*) ret, (const char*) second);
    }

    return ret;
}

void
stomp_tools_substr(const unsigned char *src, unsigned char *dest, int offset, int length) {
    memcpy(dest, src + offset, length);
}

int
stomp_tools_substr_to(const unsigned char *src, unsigned char *dest, int offset, unsigned char delim) {
    int length = 0;

    if (src != NULL && (src + offset) != NULL
            && (*(src + offset) != 0x00
            || *(src + offset) != delim)) {

        while ((src + offset + length) != NULL
                && (*(src + offset + length) != 0x00
                || *(src + offset + length) != delim)) {
            length = length + 1;
        }
    }

    stomp_tools_substr(src, dest, offset, length);

    return offset + length + 1;
}

unsigned char*
stomp_tools_long_to_bytes(const unsigned long value_long) {
    unsigned char *value_bytes = NEW_ARRAY(unsigned char, JAVA_LONG_SIZE * 2);
    int i = 0, offset = 0;

    value_bytes[offset] = (unsigned char) (value_long & 0xF);
    offset = offset + 1;

    value_bytes[offset] = (unsigned char) ((value_long >> 4) & 0xF);
    offset = offset + 1;

    for (i = 2; i < C_LONG_SIZE * 2; i = i + 2) {
        value_bytes[offset] = (unsigned char) ((value_long >> (4 * i)) & 0xF);
        offset = offset + 1;

        value_bytes[offset] = (unsigned char) ((value_long >> (4 * (i + 1))) & 0xF);
        offset = offset + 1;
    }

    return value_bytes;
}

unsigned long
stomp_tools_bytes_to_long(const unsigned char *value_bytes) {
    unsigned long value_long = 0;
    int i = 0, offset = 0;

    value_long = ((unsigned long) value_bytes[offset]);
    value_long |= ((unsigned long) value_bytes[offset + 1]) << 4;

    for (i = 2; i < C_LONG_SIZE * 2; i = i + 2) {
        value_long |= ((unsigned long) value_bytes[offset + i]) << (4 * i);
        value_long |= ((unsigned long) value_bytes[offset + i + 1]) << (4 * (i + 1));
    }

    return value_long;
}