/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_AI_H
#define CMP_AI_H

#include "cmp_ori.h"
#include "cmp_drv.h"

struct LvlGraph;
struct TilePos;

struct CmpAiTacticalStatus {
    struct PosRot hunter_pos;
};

enum CmpAiSoulState {
    CMP_AI_SOUL_STATE_IDLE,
    CMP_AI_SOUL_STATE_PANIC,
    CMP_AI_SOUL_STATE_KO,
    CMP_AI_SOUL_STATE_HANGING
};

struct CmpAiSoul {
    enum CmpAiSoulState state;
    double think_timer, think_timer_max;
};

enum CmpAiType {
    CMP_AI_SOUL
};

struct CmpAi {
    enum CmpAiType type;
    union {
        struct CmpAiSoul soul;
    } body;
};

struct CmpAi *cmp_ai_soul_create(enum CmpAiSoulState init_state);

void cmp_ai_soul_update_driver(
        struct CmpAiSoul *ai,
        struct CmpDrvWaypoint *wp,
        struct TilePos *soul_tp,
        struct LvlGraph *lgph);

void cmp_ai_free(struct CmpAi *cmp_ai);

void cmp_ai_update(
        struct CmpAi *ai,
        struct CmpDrv *drv,
        struct CmpAiTacticalStatus *ts,
        double dt);

#endif

