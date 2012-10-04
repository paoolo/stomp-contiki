#include "stomp-tools.h"

#include "stomp-memguard.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * tools.c
 *
 *  Created on: 24-03-2012
 *      Author: paoolo
 */

char*
stomp_tools_strcpy(char *str) {
    char *ret = NULL;

    if (str != NULL) {
        ret = (char*) _deref(_new_ref(sizeof(char) * (strlen(str) +1)));

        strcpy(ret, str);
    }

    return ret;
}

char*
stomp_tools_strcat(char *dst, char *src) {
    int size = -1;
    char *ret = NULL;

    if (dst == NULL && src != NULL) {
        size = sizeof(char) * strlen(src);
        ret = (char*) _deref(_new_ref(size+1));

        strcpy(ret, src);

    } else if (dst != NULL && src == NULL) {
        size = sizeof(char) * strlen(dst);
        ret = (char*) _deref(_new_ref(size+1));

        strcpy(ret, dst);

    } else if (dst != NULL && src != NULL) {
        size = sizeof(char) * (strlen(dst) + strlen(src));
        ret = (char*) _deref(_new_ref(size+1));

        strcpy(ret, dst);
        strcat(ret, src);
    }

    return ret;
}

char*
stomp_tools_strtok(char **str, char delim) {
    char *data = NULL, *tmp = NULL;
    ssize_t i = 0, len = 0;

    while (str != NULL
            && *str != NULL
            && *str + i != NULL
            && *(*str + i) != delim
            && *(*str + i) != '\0') {
        i = i + 1;
    }

    if (str != NULL
            && *str != NULL
            && *str + i != NULL
            && (*(*str + i) == delim
                            || *(*str + i) == '\0')) {

        data = (char*) _deref(_new_ref(sizeof(char) * (i + 1)));

        strncpy(data, *str, i);

        if (*(*str + i) != '\0') {
            len = strlen(*str + i + 1);

            tmp = (char*) _deref(_new_ref(sizeof(char) * (len + 1)));

            strncpy(tmp, *str + i + 1, len);

            /* FIXME It is still a big BUG!! */
            /* _del_ref(*__str); */

            *str = tmp;
        }
    }

    return data;
}

char*
stomp_tools_strncpy(char *dest, const char *src, size_t n)
{
    size_t len;

    len = strlen(src);
    strncpy(dest, src, n);

    if(len > n) {
        return dest + n;
    } else {
        return dest + len;
    }
}
