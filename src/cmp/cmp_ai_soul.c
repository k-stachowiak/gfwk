/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>
#include <string.h>

#include "diagnostics.h"
#include "memory.h"

#include "cmp_ai.h"

#include "sc_data.h"
#include "sc_pain.h"
#include "sc_graph.h"
#include "sc_soul.h"

/* Common waypoint related opretaions.
 * ===================================
 */

static double *cmp_ai_tilepos_to_worldpos_ground(
		struct TilePos *tile_pos_v,
		int points_count)
{
	int i;
	double *world_pos_v = malloc_or_die(points_count * 2 * sizeof(*world_pos_v));
	for (i = 0; i < points_count; ++i) {
		struct WorldPos wp = pos_tile_to_world_ground(tile_pos_v[i]);
		world_pos_v[2 * i + 0] = wp.x;
		world_pos_v[2 * i + 1] = wp.y;
	}
	return world_pos_v;
}

static void cmp_ai_prepend_pos(
		double x, double y,
		double **points,
		int *points_count)
{
	int old_count = *points_count;
	int new_count = old_count + 1;
	double *new_points =
		malloc_or_die(new_count * 2 * sizeof(*new_points));

	new_points[0] = x;
	new_points[1] = y;
	memcpy(new_points + 2, *points, old_count * 2 * sizeof(*new_points));

	free_or_die(*points);
	*points = new_points;
	*points_count = new_count;
}

static void cmp_ai_soul_idle_drv_end(struct CmpDrv *drv, void* ai_boxed)
{
	struct CmpAiSoul *ai = (struct CmpAiSoul*)ai_boxed;

	struct CmpOri *ori = ai->ori;
	struct Graph *lgph = ai->graph;
	struct CmpDrvWaypoint *drv_wp = (struct CmpDrvWaypoint*)drv;

	struct PosRot pr = cmp_ori_get(ori);
	struct WorldPos wp = { pr.x, pr.y };

	int src_index = lgph_find_nearest(lgph, wp),
		dst_index = lgph_find_random_skip(lgph, src_index);

	struct TilePos
		src_pos = lgph->nodes[src_index],
		dst_pos = lgph->nodes[dst_index];

	struct TilePos *tp_points;
	double *dbl_points;
	int points_count;

	lgph_dijkstra(lgph, src_pos, dst_pos, &tp_points, &points_count);
	dbl_points = cmp_ai_tilepos_to_worldpos_ground(tp_points, points_count);
	cmp_ai_prepend_pos(wp.x, wp.y, &dbl_points, &points_count);

	cmp_drv_waypoint_reset(drv_wp, dbl_points, points_count);

	free_or_die(tp_points);
}

/* Transition operations.
 * ======================
 */

static void cmp_ai_soul_goto_idle(struct CmpAiSoul *this)
{
	this->state = CMP_AI_SOUL_STATE_IDLE;
	cmp_appr_proxy_set_child(this->appr, SOUL_APPR_WALK_RIGHT);
	cmp_drv_proxy_set_child(this->drv, SOUL_DRV_WALK);
}

static void cmp_ai_soul_goto_ko(struct CmpAiSoul *this)
{
	this->state = CMP_AI_SOUL_STATE_KO;
	this->think_timer = 5.0;

	cmp_appr_proxy_set_child(this->appr, SOUL_APPR_CAUGHT);
	cmp_drv_proxy_set_child(this->drv, SOUL_DRV_STAND);
}


/* Update operation.
 * =================
 */

static void cmp_ai_soul_update_panic(void)
{
}

static void cmp_ai_soul_update_ko(struct CmpAiSoul *this, double dt)
{
	if ((this->think_timer -= dt) > 0.0) {
		return;
	}

	cmp_ai_soul_goto_idle(this);
}

static void cmp_ai_soul_update_hanging(void)
{
}

static void cmp_ai_soul_update(
        struct CmpAi *this,
        struct CmpAiTacticalStatus *ts,
        double dt)
{
    (void)ts;
    struct CmpAiSoul *derived = (struct CmpAiSoul*)this;
    switch (derived->state) {
    case CMP_AI_SOUL_STATE_IDLE:
		/* Handled asynchronously. */
        break;
    case CMP_AI_SOUL_STATE_PANIC:
        cmp_ai_soul_update_panic();
        break;
    case CMP_AI_SOUL_STATE_KO:
        cmp_ai_soul_update_ko(derived, dt);
        break;
    case CMP_AI_SOUL_STATE_HANGING:
        cmp_ai_soul_update_hanging();
        break;
    }
}

/* Pain reaction.
 * ==============
 */

static void cmp_ai_soul_on_pain(long id, PainType pt, void *ai_boxed)
{	
	if (pt == PT_ARROW) {
		struct CmpAiSoul *ai = (struct CmpAiSoul *)ai_boxed;
		cmp_ai_soul_goto_ko(ai);
	}
}

/* Lifetime management operations.
 * ===============================
 */

static void cmp_ai_soul_free(struct CmpAi *this)
{
	struct CmpAiSoul *derived = (struct CmpAiSoul*)this;
	cmp_ai_soul_deinit(derived);
	free_or_die(this);
}

void cmp_ai_soul_init(
		struct CmpAiSoul *ai,
		long id,
		struct Graph *graph,
		struct CmpOri *ori,
		struct CmpDrv *drv,
		struct CmpDrvWaypoint *drv_wp, /* TODO: Consider handling the details outside AI. */
		struct CmpAppr *appr)
{
	ai->base.free = cmp_ai_soul_free;
	ai->base.update = cmp_ai_soul_update;

	ai->graph = graph;
	ai->ori = ori;
	ai->drv = drv;
	ai->appr = appr;

	cmp_ai_soul_idle_drv_end((struct CmpDrv *)drv_wp, (void*)ai);
	cmp_drv_waypoint_on_end(drv_wp, cmp_ai_soul_idle_drv_end, (void*)ai);

	cmp_ai_soul_goto_idle(ai);

	sc_pain_callback_id_register(id, (void*)ai, cmp_ai_soul_on_pain);
}

void cmp_ai_soul_deinit(struct CmpAiSoul *ai)
{
	(void)ai;
}

struct CmpAi *cmp_ai_soul_create(
		long id,
		struct Graph *graph,
		struct CmpOri *ori,
		struct CmpDrv *drv,
		struct CmpDrvWaypoint *drv_wp,
		struct CmpAppr *appr)
{
	struct CmpAiSoul *result = malloc_or_die(sizeof(*result));
	cmp_ai_soul_init(result, id, graph, ori, drv, drv_wp, appr);
	return CMP_AI(result);
}