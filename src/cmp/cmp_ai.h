/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_AI_H
#define CMP_AI_H

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_pain.h"

struct Graph;
struct TilePos;

/* Helpers.
 * ========
 */

struct CmpAiTacticalStatus {
    struct PosRot hunter_pos;
};

/* Base class.
 * ===========
 */

struct CmpAi {
    void (*free)(struct CmpAi*);
    void (*update)(
        struct CmpAi*,
        struct CmpOri*,
        struct CmpDrv*,
        struct CmpAiTacticalStatus*,
        double);
};

#define CMP_AI(T) (struct CmpAi*)(T)

/* Soul AI.
 * ========
 */

enum CmpAiSoulState {
	CMP_AI_SOUL_STATE_IDLE,
	CMP_AI_SOUL_STATE_PANIC,
	CMP_AI_SOUL_STATE_KO,
	CMP_AI_SOUL_STATE_HANGING
};

struct CmpAiSoul {
	struct CmpAi base;
	enum CmpAiSoulState state;
	double think_timer;
	double think_timer_max;
	struct Graph *graph;
	struct CmpOri *ori;
};

void cmp_ai_soul_init(
		struct CmpAiSoul *ai,
		long id,
		struct Graph *graph,
		struct CmpOri *ori,
		struct CmpDrv *drv);

void cmp_ai_soul_deinit(struct CmpAiSoul *ai);

struct CmpAi *cmp_ai_soul_create(
		long id,
		struct Graph *graph,
		struct CmpOri *ori,
		struct CmpDrv *drv);

#endif
