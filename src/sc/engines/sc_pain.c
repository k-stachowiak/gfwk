/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "array.h"
#include "memory.h"

#include "cmp_shape.h"
#include "cmp_operations.h"

#include "sc_data.h"

#include "sc_collision.h"
#include "sc_pain.h"

#include "sc_soul.h"
#include "sc_arrow.h"
#include "sc_hunter.h"

/* Pain callbacks keyed with id.
 * =============================
 */

struct PainCallbackIdNode {
	long id;
	void *data;
	PainCallbackId callback;
	struct PainCallbackIdNode *next;
} *pain_callback_id_nodes;

static struct PainCallbackIdNode *sc_pain_callback_id_seek(long id)
{
	struct PainCallbackIdNode *current = pain_callback_id_nodes;
	while (current) {
		if (current->id == id) {
			break;
		} else {
			current = current->next;
		}
	}
	return current;
}

static void sc_pain_callback_id_insert(long id, void *data, PainCallbackId callback)
{
	struct PainCallbackIdNode *node = malloc_or_die(sizeof *node);
	node->id = id;
	node->data = data;
	node->callback = callback;
	node->next = pain_callback_id_nodes;
	pain_callback_id_nodes = node;
}

/* Pain callbacks keyed with type.
 * =============================
 */

struct PainCallbackTypeNode {
	PainType type;
	void *data;
	PainCallbackType callback;
	struct PainCallbackTypeNode *next;
} *pain_callback_type_nodes;

static struct PainCallbackTypeNode *sc_pain_callback_type_seek(PainType type)
{
	struct PainCallbackTypeNode *current = pain_callback_type_nodes;
	while (current) {
		if (current->type == type) {
			break;
		} else {
			current = current->next;
		}
	}
	return current;
}

static void sc_pain_callback_type_insert(
		PainType type,
		void *data,
		PainCallbackType callback)
{
	struct PainCallbackTypeNode *node = malloc_or_die(sizeof *node);
	node->type = type;
	node->data = data;
	node->callback = callback;
	node->next = pain_callback_type_nodes;
	pain_callback_type_nodes = node;
}

/* Pain context - debug data...
 * ============================
 */

struct PainContext {
    struct {
        struct Segment *data;
        int size, cap;
    } arrow_segs;
	struct {
		struct Circle *data;
		int size, cap;
	} soul_cirs;
	struct Circle hunter_cir;
} pc_last;

/* Main implementation.
 * ====================
 */

static void sc_pain_reset_context(
		struct Hunter *hunter,
		struct ArrowArray *arrows,
		struct SoulArray *souls)
{
	int i;

	ARRAY_FREE(pc_last.arrow_segs);
	ARRAY_FREE(pc_last.soul_cirs);

	sc_col_convert_circle_cmp(&hunter->ori, &hunter->shape.body.circle, &pc_last.hunter_cir);

	for (i = 0; i < arrows->size; ++i) {
		struct Arrow *arrow = arrows->data + i;
		struct Segment arrow_seg;
		sc_col_convert_segment_cmp(&arrow->ori, &arrow->shape.body.segment, &arrow_seg);
		ARRAY_APPEND(pc_last.arrow_segs, arrow_seg);
	}

	for (i = 0; i < souls->size; ++i) {
		struct Soul *soul = souls->data + i;
		struct Circle soul_cir;
		sc_col_convert_circle_cmp(&soul->ori, &soul->shape.body.circle, &soul_cir);
		ARRAY_APPEND(pc_last.soul_cirs, soul_cir);
	}
}

static void sc_pain_reset_conponents(
		struct Hunter *hunter,
		struct ArrowArray *arrows,
		struct SoulArray *souls)
{
	int i;

	cmp_pain_reset(&hunter->pain);

	for (i = 0; i < souls->size; ++i) {
		cmp_pain_reset(&souls->data[i].pain);
	}

	for (i = 0; i < arrows->size; ++i) {
		cmp_pain_reset(&arrows->data[i].pain);
	}
}

static void sc_pain_tick_hunter(struct Hunter *hunter, struct SoulArray *souls)
{
	int i;
	struct Circle *hunter_cir = &pc_last.hunter_cir;
	for (i = 0; i < souls->size; ++i) {
		struct Soul *soul = souls->data + i;
		struct Circle *soul_cir = pc_last.soul_cirs.data + i;
		if (sc_col_circle_circle(*hunter_cir, *soul_cir)) {
			cmp_deal_pain(&soul->pain, &hunter->pain);
		}
	}
}

static void sc_pain_tick_arrows(struct ArrowArray *arrows, struct SoulArray *souls)
{
    int i, j;
	for (i = 0; i < arrows->size; ++i) {
		struct Arrow *arrow = arrows->data + i;
		struct Segment *arrow_seg = pc_last.arrow_segs.data + i;
		for (j = 0; j < souls->size; ++j) {
			struct Soul *soul = souls->data + j;
			struct Circle *soul_cir = pc_last.soul_cirs.data + j;
			if (sc_col_segment_circle(*arrow_seg, *soul_cir)) {
				cmp_deal_pain(&soul->pain, &arrow->pain);
			}
		}
	}
}

static void sc_pain_tick_interaction(
		struct ArrowArray *arrows,
		struct SoulArray *souls,
		struct Hunter *hunter)
{
	sc_pain_reset_context(hunter, arrows, souls);
	sc_pain_reset_conponents(hunter, arrows, souls);
	sc_pain_tick_hunter(hunter, souls);
	sc_pain_tick_arrows(arrows, souls);
}

static void sc_pain_tick_feedback_common(
		PainType this_pt, long id,
		struct CmpPain *pain_cmp,
		struct PainCallbackTypeNode *type_callback)
{
	int i;
	struct PainCallbackIdNode *id_callback = sc_pain_callback_id_seek(id);
	for (i = 0; i < pain_cmp->queue_size; ++i) {
		PainType other_pt = pain_cmp->queue[i];
		if (type_callback) {
			type_callback->callback(this_pt, id, other_pt, type_callback->data);
		}
		if (id_callback) {
			id_callback->callback(id_callback->id, other_pt, id_callback->data);
		}
	}
}

static void sc_pain_tick_feedback_arrows(struct ArrowArray *arrows)
{
	int i;
	struct PainCallbackTypeNode *type_callback = sc_pain_callback_type_seek(PT_ARROW);
	for (i = 0; i < arrows->size; ++i) {
		struct Arrow *arrow = arrows->data + i;
		struct CmpPain *pain = &arrow->pain;
		long id = arrow->id;
		sc_pain_tick_feedback_common(PT_ARROW, id, pain, type_callback);
	}
}

static void sc_pain_tick_feedback_souls(struct SoulArray *souls)
{
	int i;
	struct PainCallbackTypeNode *type_callback = sc_pain_callback_type_seek(PT_SOUL);
	for (i = 0; i < souls->size; ++i) {
		struct Soul *soul = souls->data + i;
		struct CmpPain *pain = &soul->pain;
		long id = soul->id;
		sc_pain_tick_feedback_common(PT_SOUL, id, pain, type_callback);
	}
}

static void sc_pain_tick_feedback_hunter(struct Hunter *hunter)
{
	struct PainCallbackTypeNode *type_callback = sc_pain_callback_type_seek(PT_HUNTER);
	struct CmpPain *pain = &hunter->pain;
	long id = hunter->id;
	sc_pain_tick_feedback_common(PT_HUNTER, id, pain, type_callback);
}

static void sc_pain_tick_feedback(
		struct ArrowArray *arrows,
		struct SoulArray *souls,
		struct Hunter *hunter)
{
	sc_pain_tick_feedback_hunter(hunter);
	sc_pain_tick_feedback_arrows(arrows);
	sc_pain_tick_feedback_souls(souls);
}

void sc_pain_init(void)
{
	pain_callback_id_nodes = NULL;
	pain_callback_type_nodes = NULL;
}

void sc_pain_deinit(void)
{
	while (pain_callback_id_nodes) {
		struct PainCallbackIdNode* current = pain_callback_id_nodes;
		pain_callback_id_nodes = pain_callback_id_nodes->next;
		free_or_die(current);
	}

	while (pain_callback_type_nodes) {
		struct PainCallbackTypeNode* current = pain_callback_type_nodes;
		pain_callback_type_nodes = pain_callback_type_nodes->next;
		free_or_die(current);
	}
}

void sc_pain_draw_debug(void)
{
	int i;	
	sc_col_draw_circle(pc_last.hunter_cir, 1, 0.5, 0.5);
    for (i = 0; i < pc_last.arrow_segs.size; ++i) {
		sc_col_draw_segment(pc_last.arrow_segs.data[i], 0.5, 1, 0.5);
    }
	for (i = 0; i < pc_last.soul_cirs.size; ++i) {
		sc_col_draw_circle(pc_last.soul_cirs.data[i], 1, 0.5, 1);
	}
}

void sc_pain_tick(
		struct ArrowArray *arrows,
		struct SoulArray *souls,
		struct Hunter *hunter)
{
	sc_pain_tick_interaction(arrows, souls, hunter);
	sc_pain_tick_feedback(arrows, souls, hunter);
}

void sc_pain_callback_id_register(long id, void *data, PainCallbackId callback)
{
	struct PainCallbackIdNode *found = sc_pain_callback_id_seek(id);

	if (!found) {
		sc_pain_callback_id_insert(id, data, callback);
	} else {
		found->data = data;
		found->callback = callback;
	}
}

void sc_pain_callback_type_register(PainType type, void *data, PainCallbackType callback)
{
	struct PainCallbackTypeNode *found = sc_pain_callback_type_seek(type);

	if (!found) {
		sc_pain_callback_type_insert(type, data, callback);
	}
	else {
		found->data = data;
		found->callback = callback;
	}
}