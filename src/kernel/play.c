/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <allegro5/allegro_audio.h>

#include "diagnostics.h"
#include "play.h"

struct SampleIdNode {
    ALLEGRO_SAMPLE *key;
    ALLEGRO_SAMPLE_ID value;
    struct SampleIdNode *next;
};

static struct SampleIdNode *sample_id_map;

static struct SampleIdNode *sample_id_find(ALLEGRO_SAMPLE *sample)
{
    struct SampleIdNode *node = sample_id_map;
    while (node) {
        if (node->key == sample) {
            return node;
        }
        node = node->next;
    }

    return NULL;
}

static void sample_id_assign(ALLEGRO_SAMPLE *sample, ALLEGRO_SAMPLE_ID id)
{
    struct SampleIdNode *node = sample_id_find(sample);

    if (node) {
        node->value = id;
        return;
    }

    node = malloc(sizeof(*node));

    if (!node) {
        DIAG_ERROR("Allocation failed.\n");
    }

    node->key = sample;
    node->value = id;
    node->next = sample_id_map;

    sample_id_map = node;
}

void play_init(void)
{
    al_reserve_samples(16);
}

void play_sample(void *sample)
{
    ALLEGRO_SAMPLE_ID id;
    struct SampleIdNode *node = sample_id_find((ALLEGRO_SAMPLE*)sample);

    if (node) {
        al_stop_sample(&node->value);
    }

    if (!al_play_sample(
            (ALLEGRO_SAMPLE*)sample,
            1, 0, 1,
            ALLEGRO_PLAYMODE_ONCE,
            &id)) {
        return;
    }

    sample_id_assign((ALLEGRO_SAMPLE*)sample, id);
}
