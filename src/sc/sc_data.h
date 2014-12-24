/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef SC_DATA_H
#define SC_DATA_H

struct TilePos { int x, y; };
struct WorldPos { double x, y; };
struct ScreenPos { double x, y; };

struct WorldPos pos_tile_to_world(struct TilePos tile_pos);
struct WorldPos pos_tile_to_world_ground(struct TilePos tile_pos);
struct ScreenPos pos_world_to_screen(struct WorldPos world_pos);

/* Configuration cache. */
extern int sc_screen_w;
extern int sc_screen_h;
extern int sc_tile_w;

/* Basic resources. */
extern void *sc_debug_font;
extern void *sc_tile;
extern void *sc_soulbooth;
extern void *sc_hunter_stand_right;
extern void *sc_hunter_stand_left;
extern void *sc_hunter_walk_right;
extern void *sc_hunter_walk_left;
extern void *sc_bow_bitmap;
extern void *sc_arrow_bitmap;
extern void *sc_soul_stand_right;
extern void *sc_soul_stand_left;
extern void *sc_soul_walk_right;
extern void *sc_soul_walk_left;
extern void *sc_soul_caught;

/* Cmoplex resources. */
extern struct CmpApprAnimSpriteCommon *sc_hunter_walk_right_common;
extern struct CmpApprAnimSpriteCommon *sc_hunter_walk_left_common;
extern struct CmpApprAnimSpriteCommon *sc_soul_walk_right_common;
extern struct CmpApprAnimSpriteCommon *sc_soul_walk_left_common;

/* View state. */
extern struct WorldPos sc_cam_shift;

/* Entities. */
enum SCPainType {
	PT_HUNTER,
	PT_SOUL,
	PT_ARROW
};

extern struct Level lvl;
extern struct Graph lgph;
extern struct Hunter hunter;
extern struct Soul soul;

struct ArrowArray {
	struct Arrow *data;
	int size, cap;
};

extern struct ArrowArray arrows;
extern struct ArrowArray arrows_stuck;

#endif
