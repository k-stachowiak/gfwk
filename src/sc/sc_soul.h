/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "sc_data.h"
#include "sc_level.h"

struct Soul {
    struct CmpAppr *appr_stand_right;
    struct CmpAppr *appr_stand_left;
    struct CmpAppr *appr_walk_right;
    struct CmpAppr *appr_walk_left;

    struct CmpAppr *appr;
    struct CmpOri *ori;
    struct CmpDrv *drv;
    struct CmpAi *ai;

    double box_w, box_h;
};

void soul_init(struct Soul *soul, struct LvlGraph *lgph, struct TilePos tp);
void soul_deinit(struct Soul *soul);
void soul_tick(struct Soul *soul, struct CmpAiTacticalStatus *ts, double dt);
void soul_draw(struct Soul *soul);
