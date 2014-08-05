/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef MENU_STRUCT_H
#define MENU_STRUCT_H

struct MenuPage;

struct Menu {
    struct MenuPage *main_page;
    struct MenuPage *current_page;
};

struct Menu *menu_create(struct MenuPage *main_page);
void menu_free(struct Menu *menu);

struct MenuPage {
    char *caption;
    int current_item;
    struct MenuItem *items;
};

struct MenuPage *menu_page_create(
        char *caption,
        struct MenuItem *items);

void menu_page_free(struct MenuPage *menu_page);

enum MenuItemType {
    MENU_ITEM_VALUE,
    MENU_ITEM_LINK
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
    } body;
};

struct MenuItem *menu_item_create_value(
        char *caption,
        long *integer,
        double *real);

struct MenuItem *menu_item_create_ref(
        char *caption,
        struct MenuPage *ref);

void menu_item_free(struct MenuItem *menu_item);

#endif
