/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdbool.h>

#include "sc_data.h"

struct Hunter {
    struct CmpAppr *appr;
    struct CmpOri *ori;
    struct CmpDrv *drv;
    double box_w, box_h;
    int inx;
    bool jump_req;
    bool standing;
};

void hunter_init(struct Hunter *hunter, void *stand_bitmap);
void hunter_tick(struct Hunter *hunter, double dt);
void hunter_draw(struct Hunter *hunter);

