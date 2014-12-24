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
    struct Circle soul_cir;
	struct Circle hunter_cir;
} pc_last;

/* Main implementation.
 * ====================
 */

static void sc_pain_reset_soul(struct Soul *soul)
{
	cmp_pain_reset(&soul->pain);
}

static void sc_pain_reset_hunter(struct Hunter *hunter)
{
	cmp_pain_reset(&hunter->pain);
}

static void sc_pain_reset_arrows(struct ArrowArray *arrows)
{
	int i;
	for (i = 0; i < arrows->size; ++i) {
		cmp_pain_reset(&arrows->data[i].pain);
	}
}

static void sc_pain_tick_hunter(
		struct Hunter *hunter, struct Circle hunter_cir,
		struct Soul *soul, struct Circle soul_cir)
{
	if (sc_col_circle_circle(hunter_cir, soul_cir)) {
		cmp_deal_pain(&soul->pain, &hunter->pain);
	}
}

static void sc_pain_tick_arrows(
		struct ArrowArray *arrows,
        struct Soul *soul, struct Circle *soul_cir)
{
    int i;
    for (i = 0; i < arrows->size; ++i) {

		struct Arrow *arrow = arrows->data + i;
        struct Segment arrow_seg;
		sc_col_convert_segment_cmp(&arrow->ori, &arrow->shape.body.segment, &arrow_seg);
        ARRAY_APPEND(pc_last.arrow_segs, arrow_seg);

		if (sc_col_segment_circle(arrow_seg, *soul_cir)) {
			cmp_deal_pain(&soul->pain, &arrows->data[i].pain);
        }
    }
}

static void sc_pain_tick_interaction(
		struct ArrowArray *arrows,
		struct Soul *soul,
		struct Hunter *hunter)
{
	struct Circle soul_cir;
	struct Circle hunter_cir;

	ARRAY_FREE(pc_last.arrow_segs);

	sc_col_convert_circle_cmp(&soul->ori, &soul->shape.body.circle, &soul_cir);
	sc_col_convert_circle_cmp(&hunter->ori, &hunter->shape.body.circle, &hunter_cir);

	sc_pain_reset_soul(soul);
	sc_pain_reset_hunter(hunter);
	sc_pain_reset_arrows(arrows);

	sc_pain_tick_hunter(hunter, hunter_cir, soul, soul_cir);
	sc_pain_tick_arrows(arrows, soul, &soul_cir);

	pc_last.soul_cir = soul_cir;
	pc_last.hunter_cir = hunter_cir;
}

static void sc_pain_tick_feedback_arrows(struct ArrowArray *arrows)
{
	int i, j;
	struct PainCallbackTypeNode *found = sc_pain_callback_type_seek(PT_ARROW);

	if (!found) {
		return;
	}

	for (i = 0; i < arrows->size; ++i) {

		struct Arrow *arrow = arrows->data + i;
		struct CmpPain *pain = &arrow->pain;

		for (j = 0; j < pain->queue_size; ++j) {
			found->callback(PT_ARROW, arrow->id, pain->queue[j], found->data);
		}
	}
}

static void sc_pain_tick_feedback_soul(struct Soul *soul)
{
	int i;
	struct CmpPain *pain = &soul->pain;
	struct PainCallbackIdNode *found = sc_pain_callback_id_seek(soul->id);

	if (!found) {
		return;
	}

	for (i = 0; i < pain->queue_size; ++i) {
		PainType pt = pain->queue[i];
		found->callback(found->id, pt, found->data);
	}
}

static void sc_pain_tick_feedback_hunter(struct Hunter *hunter)
{
	int i;
	struct CmpPain *pain = &hunter->pain;
	struct PainCallbackIdNode *found = sc_pain_callback_id_seek(hunter->id);

	if (!found) {
		return;
	}

	for (i = 0; i < pain->queue_size; ++i) {
		PainType pt = pain->queue[i];
		found->callback(found->id, pt, found->data);
	}
}

static void sc_pain_tick_feedback(
		struct ArrowArray *arrows,
		struct Soul *soul,
		struct Hunter *hunter)
{
	sc_pain_tick_feedback_arrows(arrows);
	sc_pain_tick_feedback_soul(soul);
	sc_pain_tick_feedback_hunter(hunter);
}

void sc_pain_init(void)
{
	pain_callback_id_nodes = NULL;
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
	sc_col_draw_circle(pc_last.soul_cir, 1, 1, 1);
	sc_col_draw_circle(pc_last.hunter_cir, 1, 0.5, 0.5);
    for (i = 0; i < pc_last.arrow_segs.size; ++i) {
		sc_col_draw_segment(pc_last.arrow_segs.data[i], 0.5, 1, 0.5);
    }
}

void sc_pain_tick(
		struct ArrowArray *arrows,
		struct Soul *soul,
		struct Hunter *hunter)
{
	sc_pain_tick_interaction(arrows, soul, hunter);
	sc_pain_tick_feedback(arrows, soul, hunter);
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