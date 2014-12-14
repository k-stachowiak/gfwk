/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_AI_H
#define CMP_AI_H

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_pain.h"

struct Graph;
struct TilePos;

struct CmpAiTacticalStatus {
    struct PosRot hunter_pos;
};

struct CmpAi {
    void (*free)(struct CmpAi*);
    void (*update)(
        struct CmpAi*,
        struct CmpOri*,
        struct CmpDrv*,
        struct CmpAiTacticalStatus*,
        double);
};

struct CmpAi *cmp_ai_soul_create(struct Graph *graph);

#endif

