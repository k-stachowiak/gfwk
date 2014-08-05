/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "diagnostics.h"
#include "menu_struct.h"

struct MenuPageVisits {
    struct MenuPage **data;
    int size, cap;
};

static void menu_free_rec(
        struct MenuPageVisits *visits,
        struct MenuPage* page)
{
    struct MenuItem *item = page->items;
    struct MenuPage *duplicate = NULL;

    ARRAY_FIND(*visits, page, duplicate);
    if (duplicate) {
        return;
    } else {
        ARRAY_APPEND(*visits, page);
    }

    while (item) {
        if (item->type == MENU_ITEM_LINK) {
            menu_free_rec(visits, item->body.ref);
        }
        item = item->next;
    }
    menu_page_free(page);
}

struct Menu *menu_create(struct MenuPage *main_page)
{
    struct Menu *result = malloc(sizeof(*result));

    if (!result) {
        DIAG_ERROR("Allocation error.\n");
    }

    result->main_page = main_page;
    result->current_page = main_page;

    return result;
}

void menu_free(struct Menu *menu)
{
    struct MenuPageVisits visits = { NULL, 0, 0 };
    menu_free_rec(&visits, menu->main_page);
    free(menu);
    ARRAY_FREE(visits);
}

struct MenuPage *menu_page_create(
        char *caption,
        struct MenuItem *items)
{
    struct MenuPage *result = malloc(sizeof(*result));
    int len = strlen(caption);
    char *caption_copy = malloc(len + 1);

    if (!result || !caption) {
        DIAG_ERROR("Allocation failure.\n");
    }

    result->caption = caption_copy;
    result->current_item = 0;
    result->items = items;

    return result;
}

void menu_page_free(struct MenuPage *menu_page)
{
    free(menu_page->caption);
    menu_item_free(menu_page->items);
    free(menu_page);
}

struct MenuItem *menu_item_create_value(
        char *caption,
        long *integer,
        double *real)
{
    struct MenuItem *result = malloc(sizeof(*result));
    int len = strlen(caption);
    char *caption_copy = malloc(len + 1);

    if (!result || !caption) {
        DIAG_ERROR("Allocation failure.\n");
    }

    if ((!!integer) + (!!real) != 1) {
        DIAG_ERROR("Only one pointer can be set for value menu item.\n");
    }

    memcpy(caption_copy, caption, len + 1);

    result->type = MENU_ITEM_VALUE;
    result->caption = caption_copy;
    result->next = NULL;

    if (integer) {
        result->body.ptr.integer = integer;
    } else if (real) {
        result->body.ptr.real = real;
    } else {
        DIAG_ERROR("Unhandled value menu item case.\n");
    }

    return result;
}

struct MenuItem *menu_item_create_ref(
        char *caption,
        struct MenuPage *ref)
{
    struct MenuItem *result = malloc(sizeof(*result));
    int len = strlen(caption);
    char *caption_copy = malloc(len + 1);

    if (!result || !caption) {
        DIAG_ERROR("Allocation failure.\n");
    }

    memcpy(caption_copy, caption, len + 1);

    result->type = MENU_ITEM_LINK;
    result->caption = caption_copy;
    result->next = NULL;
    result->body.ref = ref;

    return result;
}

void menu_item_free(struct MenuItem *menu_item)
{
    free(menu_item->caption);
    if (menu_item->next) {
        menu_item_free(menu_item->next);
    }
    free(menu_item);
}

