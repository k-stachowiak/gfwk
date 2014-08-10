/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_FREE(MACRO_ARRAY) \
    do { \
        free((MACRO_ARRAY).data); \
        (MACRO_ARRAY).data = NULL; \
        (MACRO_ARRAY).size = 0; \
        (MACRO_ARRAY).cap = 0; \
    } while(0)

#define ARRAY_COPY(dst, src) \
    do { \
        int size = src.cap * sizeof(*dst.data); \
        dst.data = malloc(size); \
        memcpy(dst.data, src.data, size); \
        dst.size = src.size; \
        dst.cap = src.cap; \
    } while (0)

#define ARRAY_FIND(MACRO_ARRAY, MACRO_ELEMENT, MACRO_RESULT) \
    do { \
        int i; \
        for (i = 0; i < (MACRO_ARRAY).size; ++i) { \
            if ((MACRO_ARRAY).data[i] == (MACRO_ELEMENT)) { \
                MACRO_RESULT = (MACRO_ELEMENT); \
                break; \
            } \
        } \
    } while(0)

#define ARRAY_APPEND(MACRO_ARRAY, MACRO_ELEMENT) \
    do { \
        if ((MACRO_ARRAY).cap == 0) { \
            (MACRO_ARRAY).data = malloc(sizeof(*((MACRO_ARRAY).data))); \
            if (!(MACRO_ARRAY).data) { \
                fprintf(stderr, "Allocation failure.\n"); \
                exit(1); \
            } \
            (MACRO_ARRAY).cap = 1; \
        } else if ((MACRO_ARRAY).cap == (MACRO_ARRAY).size) { \
            (MACRO_ARRAY).cap *= 2; \
            (MACRO_ARRAY).data = realloc(\
                    (MACRO_ARRAY).data,\
                    (MACRO_ARRAY).cap * sizeof(*((MACRO_ARRAY).data))); \
            if (!(MACRO_ARRAY).data) { \
                fprintf(stderr, "Allocation failure.\n"); \
                exit(1); \
            } \
        } \
        (MACRO_ARRAY).data[(MACRO_ARRAY).size++] = (MACRO_ELEMENT); \
    } while (0)

#define ARRAY_REMOVE(MACRO_ARRAY, MACRO_INDEX) \
    do { \
        (MACRO_ARRAY).data[(MACRO_INDEX)] = (MACRO_ARRAY).data[(MACRO_ARRAY).size - 1]; \
        (MACRO_ARRAY).size -= 1; \
    } while(0)

#endif
