/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef HUNTER_H
#define HUNTER_H

#include <stdbool.h>

#include "sc_data.h"

#include "cmp_ori.h"

struct Hunter {
	struct CmpAppr *appr;
    struct CmpDrv *drv;

    struct CmpAppr *appr_stand_right;
    struct CmpAppr *appr_stand_left;
    struct CmpAppr *appr_walk_right;
    struct CmpAppr *appr_walk_left;

	struct CmpOri ori;

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

#endif
