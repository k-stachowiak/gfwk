/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>
#include <string.h>

#include "array.h"
#include "diagnostics.h"
#include "sc_data.h"

int sc_screen_w;
int sc_screen_h;
int sc_tile_w;

void *sc_debug_font;
void *sc_tile;
void *sc_soulbooth;
void *sc_hunter_stand_right;
void *sc_hunter_stand_left;
void *sc_hunter_walk_right;
void *sc_hunter_walk_left;
void *sc_bow_bitmap;
void *sc_arrow_bitmap;
void *sc_soul_stand_right;
void *sc_soul_stand_left;
void *sc_soul_walk_right;
void *sc_soul_walk_left;

struct CmpApprAnimSpriteCommon *sc_hunter_walk_right_common;
struct CmpApprAnimSpriteCommon *sc_hunter_walk_left_common;
struct CmpApprAnimSpriteCommon *sc_soul_walk_right_common;
struct CmpApprAnimSpriteCommon *sc_soul_walk_left_common;

struct WorldPos sc_cam_shift;

struct WorldPos pos_tile_to_world(struct TilePos tile_pos)
{
    struct WorldPos result = {
        tile_pos.x * sc_tile_w,
        tile_pos.y * sc_tile_w
    };
    return result;
}

struct ScreenPos pos_world_to_screen(struct WorldPos world_pos)
{
    struct ScreenPos result = {
        world_pos.x - sc_cam_shift.x + sc_screen_w / 2.0,
        world_pos.y - sc_cam_shift.y + sc_screen_h / 2.0
    };
    return result;
}

