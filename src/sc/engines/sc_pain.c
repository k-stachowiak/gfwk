/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "array.h"
#include "memory.h"

#include "cmp_operations.h"

#include "sc_data.h"

#include "sc_collision.h"
#include "sc_pain.h"

#include "sc_soul.h"
#include "sc_arrow.h"

struct PainCallbackNode {
	long id;
	void *data; /* TODO: drop data and force client to map data to id ??? */
	PainCallback callback;
	struct PainCallbackNode *next;
} *pain_callback_nodes;

static struct PainCallbackNode *sc_pain_callback_seek(long id)
{
	struct PainCallbackNode *current = pain_callback_nodes;
	while (current) {
		if (current->id == id) {
			break;
		}
	}
	return current;
}

static void sc_pain_callback_insert(long id, void *data, PainCallback callback)
{
	struct PainCallbackNode *node = malloc_or_die(sizeof *node);
	node->id = id;
	node->data = data;
	node->callback = callback;
	node->next = pain_callback_nodes;
	pain_callback_nodes = node;
}

struct PainContext {
    struct {
        struct Segment *data;
        int size, cap;
    } arrow_segs;
    struct Circle soul_cir;
} pc_last;

static void pain_reset_soul(struct Soul *soul)
{
	cmp_pain_reset(&soul->pain);
}

static void pain_tick_soul(
		struct Soul *soul,
		struct PosRot *soul_pr,
		struct Circle *soul_cir)
{
	*soul_pr = cmp_ori_get(&soul->ori);
	soul_cir->x = soul_pr->x;
	soul_cir->y = soul_pr->y;
	soul_cir->r = 25.0;
}

static void pain_reset_arrows(struct ArrowArray *arrows)
{
	int i;
	for (i = 0; i < arrows->size; ++i) {
		cmp_pain_reset(&arrows->data[i].pain);
	}
}

static void pain_tick_arrows(
		struct ArrowArray *arrows,
        struct Soul *soul, struct Circle *soul_cir)
{
    int i;
    for (i = 0; i < arrows->size; ++i) {

		struct PosRot arrow_pr = cmp_ori_get(&arrows->data[i].ori);
        struct Segment arrow_seg;

        arrow_seg.ax = arrow_pr.x;
        arrow_seg.ay = arrow_pr.y;
        arrow_seg.bx = arrow_pr.x + 25 * cos(arrow_pr.theta);
        arrow_seg.by = arrow_pr.y + 25 * sin(arrow_pr.theta);

        ARRAY_APPEND(pc_last.arrow_segs, arrow_seg);

        if (col_segment_circle(arrow_seg, *soul_cir)) {
			cmp_deal_pain(&soul->pain, &arrows->data[i].pain);
        }
    }
}

static void pain_tick_interaction(struct ArrowArray *arrows, struct Soul *soul)
{
	struct PosRot soul_pr;
	struct Circle soul_cir;

	ARRAY_FREE(pc_last.arrow_segs);

	pain_reset_soul(soul);
	pain_reset_arrows(arrows);

	pain_tick_soul(soul, &soul_pr, &soul_cir);
	pain_tick_arrows(arrows, soul, &soul_cir);

	pc_last.soul_cir = soul_cir;
}

static void pain_tick_feedback_arrows(
		struct ArrowArray *arrows,
		struct ArrowArray *arrows_stuck)
{
	/* TODO: Handle with a callback as well. */
	int i, j;
	for (i = 0; i < arrows->size; ++i) {

		struct Arrow *arrow = arrows->data + i;
		struct CmpPain *pain = &arrow->pain;

		for (j = 0; j < pain->queue_size; ++j) {
			if (pain->queue[j] == PT_SOUL) {
				arrow->timer = 1.0;
				ARRAY_APPEND(*arrows_stuck, *arrow);
				ARRAY_REMOVE(*arrows, i);
				--i;
				break;
			}
		}
	}
}

static void pain_tick_feedback_soul(struct Soul *soul)
{
	int i;
	struct CmpPain *pain = &soul->pain;
	for (i = 0; i < pain->queue_size; ++i) {
		if (pain->queue[i] == PT_ARROW) {
			struct PainCallbackNode *found = sc_pain_callback_seek(soul->id);
			if (found) {
				found->callback(found->id, found->data);
			}
		}
	}
}

static void pain_tick_feedback(
		struct ArrowArray *arrows,
		struct ArrowArray *arrows_stuck,
		struct Soul *soul)
{
	pain_tick_feedback_arrows(arrows, arrows_stuck);
	pain_tick_feedback_soul(soul);
}



void sc_pain_init(void)
{
	pain_callback_nodes = NULL;
}

void sc_pain_deinit(void)
{
	while (pain_callback_nodes) {
		struct PainCallbackNode* current = pain_callback_nodes;
		pain_callback_nodes = pain_callback_nodes->next;
		free_or_die(current);
	}
}

void pain_draw_debug(void)
{
	int i;
	col_draw_circle(pc_last.soul_cir, 1, 1, 1);
    for (i = 0; i < pc_last.arrow_segs.size; ++i) {
        col_draw_segment(pc_last.arrow_segs.data[i], 1, 1, 1);
    }
}

void pain_tick(
		struct ArrowArray *arrows,
		struct ArrowArray *arrows_stuck,
		struct Soul *soul)
{
	pain_tick_interaction(arrows, soul);
	pain_tick_feedback(arrows, arrows_stuck, soul);
}

void sc_pain_callback_register(long id, void *data, PainCallback callback)
{
	struct PainCallbackNode *found = sc_pain_callback_seek(id);

	if (!found) {
		sc_pain_callback_insert(id, data, callback);
	} else {
		found->callback = callback;
	}
}