/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>

#include <allegro5/allegro.h>

struct SysClient {
    bool *alive;
    struct SysClient **next;
    void (*init)(void);
    void (*deinit)(void);
    void (*tick)(double);
    void (*draw)(double);
    void (*key)(int, bool);
};

extern bool sys_keys[ALLEGRO_KEY_MAX];

void sys_init(void);
void sys_realtime_loop(struct SysClient *client);
void sys_deinit(void);

#endif
