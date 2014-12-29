/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_AI_H
#define CMP_AI_H

#include "cmp_ori.h"
#include "cmp_drv.h"
#include "cmp_pain.h"

enum CmpAiType {
	CMP_AI_SOUL
};

struct Graph;
struct TilePos;

/* Helpers.
 * ========
 */

struct CmpAiTacticalStatus {
    struct PosRot hunter_pos;
};

/* Soul AI.
 * ========
 */

enum CmpAiSoulState {
	CMP_AI_SOUL_STATE_INVALID,
	CMP_AI_SOUL_STATE_IDLE,
	CMP_AI_SOUL_STATE_PANIC,
	CMP_AI_SOUL_STATE_KO
};

struct CmpAiSoul {
	enum CmpAiSoulState state;
	enum CmpAiSoulState next_state;
	double think_timer;
	struct PosRot last_pr;
	struct Graph *graph;
};

/* Root struct. 
 * ============
 */

struct CmpAi {
	union {
		struct CmpAiSoul soul;
	} body;
	enum CmpAiType type;
};

void cmp_ai_soul_init(
		struct CmpAi *ai,
		long id,
		struct Graph *graph);

void cmp_ai_deinit(struct CmpAi *ai);

void cmp_ai_update(
		struct CmpAi *ai,
		struct CmpOri *ori,
		struct CmpDrv *drv,
		struct CmpAppr *appr,
		struct CmpAiTacticalStatus* ts,
		double dt);

#endif
