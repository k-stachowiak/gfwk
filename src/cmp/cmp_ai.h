/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_AI_H
#define CMP_AI_H

#include "cmp_ori.h"
#include "cmp_drv.h"

struct Graph;
struct TilePos;

struct CmpAiTacticalStatus {
    struct PosRot hunter_pos;
};

struct CmpAi {
    void (*free)(struct CmpAi*);
    void (*update)(struct CmpAi*, struct CmpDrv*drv, struct CmpAiTacticalStatus*, double);
    void (*update_driver)(struct CmpAi*, struct CmpDrv*, struct TilePos*, struct Graph*);
};

struct CmpAi *cmp_ai_soul_create(void);

#endif

