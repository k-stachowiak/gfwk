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
    result->nrb_top = 0;
    result->nrb_bottom = 0;

    return result;
}

void menu_free(struct Menu *menu)
{
    struct MenuPageVisits visits = { NULL, 0, 0 };
    menu_free_rec(&visits, menu->main_page);
    free(menu);
    ARRAY_FREE(visits);
}

void menu_nav_up(struct Menu *menu)
{
    int items_count = menu_page_items_count(menu->current_page);
    if ((menu->current_page->current_item) == 0) {
        menu->current_page->current_item = items_count;
    }
    --menu->current_page->current_item;
}

void menu_nav_down(struct Menu *menu)
{
    int items_count = menu_page_items_count(menu->current_page);
    if ((++menu->current_page->current_item) == items_count) {
        menu->current_page->current_item = 0;
    }
}

bool menu_nav_enter(struct Menu *menu)
{
    struct MenuItem *item = menu_page_current_item(menu->current_page);

    switch (item->type) {
    case MENU_ITEM_VALUE:
        return false;

    case MENU_ITEM_LINK:
        menu->nav_ring_buff[menu->nrb_top] = menu->current_page;
        menu->nrb_top += 1;
        menu->nrb_top %= MENU_NRB_SIZE;
        if (menu->nrb_top == menu->nrb_bottom) {
            menu->nrb_bottom += 1;
            menu->nrb_bottom %= MENU_NRB_SIZE;
        }
        menu->current_page = item->body.ref;

        break;

    case MENU_ITEM_ACTION:
        item->body.callback();
        break;
    }
    return true;
}

bool menu_nav_back(struct Menu *menu)
{
    if (menu->nrb_top == menu->nrb_bottom) {
        return false;
    }

    menu->nrb_top -= 1;
    menu->nrb_top %= MENU_NRB_SIZE;

    menu->current_page = menu->nav_ring_buff[menu->nrb_top];

    return true;
}

bool menu_input_incr(struct Menu *menu)
{
    struct MenuItem *item = menu_page_current_item(menu->current_page);

    switch (item->type) {
    case MENU_ITEM_VALUE:
        if (item->body.ptr.integer) {
            *(item->body.ptr.integer) += 1;
        } else if (item->body.ptr.real) {
            *(item->body.ptr.real) += 1.0;
        } else {
            DIAG_ERROR("Unhandled menu value type.\n");
        }
        break;

    case MENU_ITEM_LINK:
    case MENU_ITEM_ACTION:
        return false;
    }

    return true;
}

bool menu_input_decr(struct Menu *menu)
{
    struct MenuItem *item = menu_page_current_item(menu->current_page);

    switch (item->type) {
    case MENU_ITEM_VALUE:
        if (item->body.ptr.integer) {
            *(item->body.ptr.integer) -= 1;
        } else if (item->body.ptr.real) {
            *(item->body.ptr.real) -= 1.0;
        } else {
            DIAG_ERROR("Unhandled menu value type.\n");
        }
        break;

    case MENU_ITEM_LINK:
    case MENU_ITEM_ACTION:
        return false;
    }

    return true;
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

    memcpy(caption_copy, caption, len + 1);

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

int menu_page_items_count(struct MenuPage *menu_page)
{
    int items_count = 0;
    struct MenuItem *item = menu_page->items;

    while (item) {
        ++items_count;
        item = item->next;
    }

    return items_count;
}

struct MenuItem *menu_page_current_item(struct MenuPage *menu_page)
{
    int index = 0;
    struct MenuItem *item = menu_page->items;

    while (item) {
        if (index++ == menu_page->current_item) {
            return item;
        }
        item = item->next;
    }

    DIAG_ERROR(
        "Data corruption - current item (%d) out of item list bounds.\n",
        menu_page->current_item);
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

struct MenuItem *menu_item_create_action(
        char *caption,
        void (*callback)(void))
{
    struct MenuItem *result = malloc(sizeof(*result));
    int len = strlen(caption);
    char *caption_copy = malloc(len + 1);

    if (!result || !caption) {
        DIAG_ERROR("Allocation failure.\n");
    }

    memcpy(caption_copy, caption, len + 1);

    result->type = MENU_ITEM_ACTION;
    result->caption = caption_copy;
    result->next = NULL;
    result->body.callback = callback;

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

