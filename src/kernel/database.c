/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "diagnostics.h"
#include "memory.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct ConfigNode {
    char *key;
    union { double d; long l; } value;
    struct ConfigNode *next;
};

static struct ConfigNode *integers;
static struct ConfigNode *reals;

static struct ConfigNode *db_make_node(char *key)
{
    int key_len;
    char *key_copy;
    struct ConfigNode *new_node;

    key_len = strlen(key);
    key_copy = malloc_or_die(key_len + 1);
    memcpy(key_copy, key, key_len + 1);

    new_node = malloc_or_die(sizeof(*new_node));
    new_node->key = key_copy;
    
    return new_node;
}

static struct ConfigNode *db_find(struct ConfigNode *list, char *key)
{
    struct ConfigNode *current;
    for (current = list; current; current = current->next) {
        if (strcmp(key, current->key) == 0) {
            return current;
        }
    }
    return NULL;
}

static void db_free_list(struct ConfigNode *list)
{
    struct ConfigNode *temp;
    while (list) {
        temp = list;
        list = list->next;
        free(temp->key);
        free(temp);
    }
}

static void db_insert_integer(char *key, long value)
{
    struct ConfigNode *new_node = db_make_node(key);
    new_node->value.l = value;
    new_node->next = integers;
    integers = new_node;
}

static void db_insert_real(char *key, double value)
{
    struct ConfigNode *new_node = db_make_node(key);
    new_node->value.d = value;
    new_node->next = reals;
    reals = new_node;
}

void db_init(void)
{
    db_insert_integer("screen_w", 800);
    db_insert_integer("screen_h", 800);

    db_insert_real("fps", 50.0);

    db_insert_real("menu_text_caption_r", 0.777);
    db_insert_real("menu_text_caption_g", 0.888);
    db_insert_real("menu_text_caption_b", 0.999);
    db_insert_real("menu_text_on_r", 0.667);
    db_insert_real("menu_text_on_g", 0.5);
    db_insert_real("menu_text_on_b", 0.333);
    db_insert_real("menu_text_off_r", 0.333);
    db_insert_real("menu_text_off_g", 0.5);
    db_insert_real("menu_text_off_b", 0.667);
    db_insert_integer("menu_text_size", 20);
    db_insert_real("menu_text_y_offset", 30.0);
}

void db_deinit(void)
{
    db_free_list(integers);
    db_free_list(reals);
}

long db_integer(char *key)
{
    struct ConfigNode *found = db_find(integers, key);
    if (!found) {
        DIAG_ERROR("Invalid key \"%s\" requested in INTEGERS.", key);
        exit(1);
    } else {
        return found->value.l;
    }
}

double db_real(char *key)
{
    struct ConfigNode *found = db_find(reals, key);
    if (!found) {
        DIAG_ERROR("Invalid key \"%s\" requested in REALS.", key);
        exit(1);
    } else {
        return found->value.d;
    }
}

