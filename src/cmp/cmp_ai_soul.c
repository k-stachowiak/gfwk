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

static void cmp_ai_soul_update_wp_drv(struct CmpAiSoul *ai, struct CmpDrv *drv)
{
	struct Graph *lgph = ai->graph;

	struct PosRot pr = ai->last_pr;
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

	cmp_drv_waypoint_reset(drv, dbl_points, points_count);

	free_or_die(tp_points);
}

/* State transitions.
 * ==================
 */

static void cmp_ai_soul_goto_idle(
		struct CmpAiSoul *ai,
		struct CmpAppr *appr,
		struct CmpDrv *drv)
{
	ai->state = CMP_AI_SOUL_STATE_IDLE;
	soul_set_appr_stand_right(appr);

	cmp_drv_proxy_set_child(drv, SOUL_DRV_WALK);
	cmp_ai_soul_update_wp_drv(ai, drv);
}

static void cmp_ai_soul_goto_ko(
		struct CmpAiSoul *ai,
		struct CmpAppr *appr,
		struct CmpDrv *drv)
{
	ai->state = CMP_AI_SOUL_STATE_KO;
	ai->think_timer = 5.0;

	soul_set_appr_caught(appr);
	cmp_drv_proxy_set_child(drv, SOUL_DRV_STAND);
}

/* Event callbacks.
 * ================
 */

static void cmp_ai_soul_on_pain(long id, PainType pt, void *data)
{
	if (pt == PT_ARROW) {
		struct CmpAiSoul *ai = (struct CmpAiSoul *)data;
		ai->next_state = CMP_AI_SOUL_STATE_KO;
	}
}

static void cmp_ai_soul_on_wp_drv_end(struct CmpDrv *drv, void* data)
{
	struct CmpAiSoul *ai = (struct CmpAiSoul*)data;
	cmp_ai_soul_update_wp_drv(ai, drv);
}

/* Update operation.
 * =================
 */

static void cmp_ai_soul_update_panic(void)
{
}

static void cmp_ai_soul_update_ko(
		struct CmpAiSoul *ai,
		struct CmpAppr *appr,
		struct CmpDrv *drv,
		double dt)
{
	if ((ai->think_timer -= dt) <= 0.0) {
		ai->next_state = CMP_AI_SOUL_STATE_IDLE;
	}
}

static void cmp_ai_update_soul(
		struct CmpAiSoul *ai,
		struct CmpOri *ori,
		struct CmpAppr *appr,
		struct CmpDrv *drv,
		double dt)
{
	ai->last_pr = cmp_ori_get(ori);
	if (ai->state == ai->next_state) {
		switch (ai->state) {
		case CMP_AI_SOUL_STATE_IDLE:
			/* Handled asynchronously. */
			break;
		case CMP_AI_SOUL_STATE_PANIC:
			cmp_ai_soul_update_panic();
			break;
		case CMP_AI_SOUL_STATE_KO:
			cmp_ai_soul_update_ko(ai, appr, drv, dt);
			break;
		}
	} else {
		switch (ai->next_state) {
		case CMP_AI_SOUL_STATE_IDLE:
			cmp_ai_soul_goto_idle(ai, appr, drv);
			break;
		case CMP_AI_SOUL_STATE_KO:
			cmp_ai_soul_goto_ko(ai, appr, drv);
			break;
		case CMP_AI_SOUL_STATE_PANIC:
			break;
		}
	}
}

/* Public API.
 * ===========
 */

void cmp_ai_soul_init(
		struct CmpAi *ai,
		long id,
		struct Graph *graph)
{
	ai->type = CMP_AI_SOUL;
	ai->body.soul.state = CMP_AI_SOUL_STATE_INVALID;
	ai->body.soul.next_state = CMP_AI_SOUL_STATE_IDLE;
	ai->body.soul.graph = graph;
	sc_pain_callback_id_register(id, (void*)(&ai->body.soul), cmp_ai_soul_on_pain);
}

void cmp_ai_deinit(struct CmpAi *ai)
{
	(void)ai;
}

void cmp_ai_update(
		struct CmpAi *ai,
		struct CmpOri *ori,
		struct CmpDrv *drv,
		struct CmpAppr *appr,
		struct CmpAiTacticalStatus* ts,
		double dt)
{
	switch (ai->type) {
	case CMP_AI_SOUL:
		cmp_ai_update_soul(&ai->body.soul, ori, appr, drv, dt);
		break;
	}
}