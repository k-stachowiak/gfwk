/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef MENU_STRUCT_H
#define MENU_STRUCT_H

#include <stdbool.h>

#define MENU_NRB_SIZE 10

struct Menu {
    struct MenuPage *main_page;
    struct MenuPage *current_page;

    struct MenuPage *nav_ring_buff[MENU_NRB_SIZE];
    int nrb_top, nrb_bottom;
};

struct Menu *menu_create(struct MenuPage *main_page);
void menu_free(struct Menu *menu);

void menu_nav_up(struct Menu *menu);
void menu_nav_down(struct Menu *menu);
bool menu_nav_enter(struct Menu *menu);
bool menu_nav_back(struct Menu *menu);
bool menu_input_incr(struct Menu *menu);
bool menu_input_decr(struct Menu *menu);

struct MenuPage {
    char *caption;
    int current_item;
    struct MenuItem *items;
};

struct MenuPage *menu_page_create(
        char *caption,
        struct MenuItem *items);

void menu_page_free(struct MenuPage *menu_page);

int menu_page_items_count(struct MenuPage *menu_page);
struct MenuItem *menu_page_current_item(struct MenuPage *menu_page);

enum MenuItemType {
    MENU_ITEM_VALUE,
    MENU_ITEM_LINK,
    MENU_ITEM_ACTION
};

struct MenuItem {
    enum MenuItemType type;
    char *caption;
    struct MenuItem *next;
    union {
        union {
            long *integer;
            double *real;
        } ptr;
        struct MenuPage *ref;
        void (*callback)(void);
    } body;
};

struct MenuItem *menu_item_create_value(
        char *caption,
        long *integer,
        double *real);

struct MenuItem *menu_item_create_ref(
        char *caption,
        struct MenuPage *ref);

struct MenuItem *menu_item_create_action(
        char *caption,
        void (*callback)(void));

void menu_item_free(struct MenuItem *menu_item);

#endif
