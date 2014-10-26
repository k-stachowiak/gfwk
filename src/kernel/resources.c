/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdlib.h>

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>

#include "diagnostics.h"
#include "resources.h"

struct ResNodeBitmap {
    char *path;
    ALLEGRO_BITMAP *value;
    struct ResNodeBitmap *next;
};

struct ResNodeSample {
    char *path;
    ALLEGRO_SAMPLE *value;
    struct ResNodeSample *next;
};

struct ResNodeFont {
    char *path;
    int size;
    ALLEGRO_FONT *value;
    struct ResNodeFont *next;
};

static struct ResNodeBitmap *bitmaps;
static struct ResNodeSample *samples;
static struct ResNodeFont *fonts;

static struct ResNodeBitmap *res_find_bitmap(char *path)
{
    struct ResNodeBitmap *current = bitmaps;
    while (current) {
        if (strcmp(current->path, path) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static struct ResNodeSample *res_find_sample(char *path)
{
    struct ResNodeSample *current = samples;
    while (current) {
        if (strcmp(current->path, path) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static struct ResNodeFont *res_find_font(char *path, int size)
{
    struct ResNodeFont *current = fonts;
    while (current) {
        if (strcmp(current->path, path) == 0 && current->size == size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void res_deinit(void)
{
    struct ResNodeBitmap *btemp;
    struct ResNodeFont *ftemp;
    struct ResNodeSample *stemp;

    while (bitmaps) {
        DIAG_WARNING("Uninitialized bitmap (%s)", bitmaps->path);
        btemp = bitmaps->next;
        free(bitmaps->path);
        al_destroy_bitmap(bitmaps->value);
        free(bitmaps);
        bitmaps = btemp;
    }

    while (samples) {
        DIAG_WARNING("Uninitialized sample (%s)", samples->path);
        stemp = samples->next;
        free(samples->path);
        al_destroy_sample(samples->value);
        free(samples);
        samples = stemp;
    }

    while (fonts) {
        DIAG_WARNING("Uninitialized font (%s)", fonts->path);
        ftemp = fonts->next;
        free(fonts->path);
        al_destroy_font(fonts->value);
        free(fonts);
        fonts = ftemp;
    }
}

void *res_load_bitmap(char *path)
{
    int path_len;
    char *path_copy;
    struct ResNodeBitmap *duplicate;
    struct ResNodeBitmap *new_node;
    ALLEGRO_BITMAP *value;

    if ((duplicate = res_find_bitmap(path))) {
        return (void*)duplicate->value;
    }

    path_len = strlen(path);
    path_copy = malloc(path_len + 1);
    new_node = malloc(sizeof(*new_node));

    if (!new_node || !path_copy) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    if (!(value = al_load_bitmap(path))) {
        DIAG_ERROR("Failed loading bitmap \"%s\".", path);
        exit(1);
    }

    memcpy(path_copy, path, path_len + 1);

    new_node->path = path_copy;
    new_node->value = value;
    new_node->next = bitmaps;
    bitmaps = new_node;

    return (void*)value;
}

void res_dispose_bitmap(void *bitmap)
{
    struct ResNodeBitmap *prev, *curr;

    if (bitmaps->value == (ALLEGRO_BITMAP*)bitmap) {
        struct ResNodeBitmap *temp;
        al_destroy_bitmap(bitmaps->value);
        temp = bitmaps;
        bitmaps = bitmaps->next;
        free(temp);
        return;
    }

    prev = bitmaps;
    curr = bitmaps->next;

    while (curr) {
        if (curr->value == (ALLEGRO_BITMAP*)bitmap) {
            al_destroy_bitmap(curr->value);
            prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    DIAG_ERROR("Failed finding bitmap to destroy.");
    exit(1);
}

void res_cut_frame_sheet(
        void *bitmap, int frame_w,
        void ***frames, int *frames_count)
{
    int i;
    int bitmap_w = al_get_bitmap_width((ALLEGRO_BITMAP*)bitmap);
    int bitmap_h = al_get_bitmap_height((ALLEGRO_BITMAP*)bitmap);

    *frames_count = bitmap_w / frame_w;
    *frames = malloc(*frames_count * sizeof(**frames));

    if (!(*frames)) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    for(i = 0; i < *frames_count; ++i) {
        ALLEGRO_BITMAP* frame_bitmap = al_create_sub_bitmap(
            (ALLEGRO_BITMAP*)bitmap, i * frame_w, 0, frame_w, bitmap_h);
        if (!frame_bitmap) {
            DIAG_ERROR("Failed creating sub-bitmap from spritesheet.");
            exit(1);
        }
        (*frames)[i] = frame_bitmap;
    }
}

void res_dispose_frame_sheet(void **frames, int frames_count)
{
    int i;
    for (i = 0; i < frames_count; ++i) {
        al_destroy_bitmap((ALLEGRO_BITMAP*)frames[i]);
    }
    free(frames);
}

void *res_load_sample(char *path)
{
    int path_len;
    char *path_copy;
    struct ResNodeSample *duplicate;
    struct ResNodeSample *new_node;
    ALLEGRO_SAMPLE *value;

    if ((duplicate = res_find_sample(path))) {
        return (void*)duplicate->value;
    }

    path_len = strlen(path);
    path_copy = malloc(path_len + 1);
    new_node = malloc(sizeof(*new_node));

    if (!new_node || !path_copy) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    if (!(value = al_load_sample(path))) {
        DIAG_ERROR("Failed loading sample \"%s\".", path);
        exit(1);
    }

    memcpy(path_copy, path, path_len + 1);

    new_node->path = path_copy;
    new_node->value = value;
    new_node->next = samples;
    samples = new_node;

    return (void*)value;
}

void res_dispose_sample(void *sample)
{
    struct ResNodeSample *prev, *curr;

    if (samples->value == (ALLEGRO_SAMPLE*)sample) {
        struct ResNodeSample *temp;
        al_destroy_sample(samples->value);
        temp = samples;
        samples = samples->next;
        free(temp);
        return;
    }

    prev = samples;
    curr = samples->next;

    while (curr) {
        if (curr->value == (ALLEGRO_SAMPLE*)sample) {
            al_destroy_sample(curr->value);
            prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    DIAG_ERROR("Failed finding sample to destroy.");
    exit(1);
}

void *res_load_font(char *path, int size)
{
    int path_len;
    char *path_copy;
    struct ResNodeFont *duplicate;
    struct ResNodeFont *new_node;
    ALLEGRO_FONT *value;

    if ((duplicate = res_find_font(path, size))) {
        return (void*)duplicate->value;
    }

    path_len = strlen(path);
    path_copy = malloc(path_len + 1);
    new_node = malloc(sizeof(*new_node));

    if (!new_node || !path_copy) {
        DIAG_ERROR("Allocation failure.");
        exit(1);
    }

    if (!(value = al_load_font(path, -size, 0))) {
        DIAG_ERROR("Failed loading font \"%s\".", path);
        exit(1);
    }

    memcpy(path_copy, path, path_len + 1);

    new_node->path = path_copy;
    new_node->size = size;
    new_node->value = value;
    new_node->next = fonts;
    fonts = new_node;

    return (void*)value;
}

void res_dispose_font(void *font)
{
    struct ResNodeFont *prev, *curr;

    if (fonts->value == (ALLEGRO_FONT*)font) {
        struct ResNodeFont *temp;
        al_destroy_font(fonts->value);
        temp = fonts;
        fonts = fonts->next;
        free(temp);
        return;
    }

    prev = fonts;
    curr = fonts->next;

    while (curr) {
        if (curr->value == (ALLEGRO_FONT*)font) {
            al_destroy_font(curr->value);
            prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    DIAG_ERROR("Failed finding font to destroy.");
    exit(1);
}
