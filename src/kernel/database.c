/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "diagnostics.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct ConfigNodeInt {
    char *key;
    long value;
    struct ConfigNodeInt *next;
};

struct ConfigNodeReal {
    char *key;
    double value;
    struct ConfigNodeReal *next;
};

static struct ConfigNodeInt *integers;
static struct ConfigNodeReal *reals;

static void db_insert_integer(char *key, long value)
{
    int key_len;
    char *key_copy;
    struct ConfigNodeInt *new_node;

    key_len = strlen(key);
    key_copy = malloc(key_len + 1);
    if (!key_copy) {
        DIAG_ERROR("Allocation failed.");
        exit(1);
    }
    memcpy(key_copy, key, key_len + 1);

    new_node = malloc(sizeof(*new_node));
    if (!new_node) {
        DIAG_ERROR("Allocation failed.");
        exit(1);
    }

    new_node->key = key_copy;
    new_node->value = value;
    new_node->next = integers;
    integers = new_node;
}

static void db_insert_real(char *key, double value)
{
    int key_len;
    char *key_copy;
    struct ConfigNodeReal *new_node;

    key_len = strlen(key);
    key_copy = malloc(key_len + 1);
    if (!key_copy) {
        DIAG_ERROR("Allocation failed.");
        exit(1);
    }
    memcpy(key_copy, key, key_len + 1);

    new_node = malloc(sizeof(*new_node));
    if (!new_node) {
        DIAG_ERROR("Allocation failed.");
        exit(1);
    }

    new_node->key = key_copy;
    new_node->value = value;
    new_node->next = reals;
    reals = new_node;
}

void db_init(void)
{
    db_insert_integer("screen_w", 800);
    db_insert_integer("screen_h", 800);
    db_insert_real("fps", 50.0);
}

void db_deinit(void)
{
    struct ConfigNodeInt *itemp;
    struct ConfigNodeReal *rtemp;

    while (integers) {
        itemp = integers->next;
        free(integers->key);
        free(integers);
        integers = itemp;
    }

    while (reals) {
        rtemp = reals->next;
        free(reals->key);
        free(reals);
        reals = rtemp;
    }
}


long db_integer(char *key)
{
    struct ConfigNodeInt *current;
    for (current = integers; current; current = current->next) {
        if (strcmp(key, current->key) == 0) {
            return current->value;
        }
    }

    DIAG_ERROR("Invalid key requested.");
    exit(1);
}

double db_real(char *key)
{
    struct ConfigNodeReal *current;
    for (current = reals; current; current = current->next) {
        if (strcmp(key, current->key) == 0) {
            return current->value;
        }
    }

    DIAG_ERROR("Invalid key requested.");
    exit(1);
}

