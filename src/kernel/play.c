/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <allegro5/allegro_audio.h>

#include "diagnostics.h"
#include "memory.h"
#include "play.h"

struct SampleIdNode {
    ALLEGRO_SAMPLE *key;
    ALLEGRO_SAMPLE_ID id;
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
        node->id = id;
        return;
    }

    node = malloc_or_die(sizeof(*node));

    node->key = sample;
    node->id = id;
    node->next = sample_id_map;

    sample_id_map = node;
}

void play_init(void)
{
    al_reserve_samples(5);
}

void play_deinit(void)
{
    struct SampleIdNode *temp;
    while (sample_id_map) {
        temp = sample_id_map->next;
        free(sample_id_map);
        sample_id_map = temp;
    }
}

void play_sample(void *sample)
{
    ALLEGRO_SAMPLE_ID id;
    struct SampleIdNode *node = sample_id_find((ALLEGRO_SAMPLE*)sample);

    if (node) {
        al_stop_sample(&node->id);
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
