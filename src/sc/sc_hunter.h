/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef HUNTER_H
#define HUNTER_H

#include <stdbool.h>

#include "sc_data.h"

struct Hunter {
    struct CmpAppr *appr;
    struct CmpOri *ori;
    struct CmpDrv *drv;

    struct CmpAppr *appr_stand_right;
    struct CmpAppr *appr_stand_left;
    struct CmpAppr *appr_walk_right;
    struct CmpAppr *appr_walk_left;

    double aim_angle;
    double box_w, box_h;
    int inx;
    bool jump_req;
    bool standing;
};

void hunter_init(struct Hunter *hunter);
void hunter_deinit(struct Hunter *hunter);
void hunter_tick(struct Hunter *hunter, double dt);
void hunter_draw(struct Hunter *hunter);

struct Arrow {
    struct CmpAppr *appr;
    struct CmpOri *ori;
    struct CmpDrv *drv;
};

void arrow_init(
        struct Arrow *arrow,
        double x, double y, double angle,
        void *arrow_bitmap);

void arrow_deinit(struct Arrow *arrow);

#endif
