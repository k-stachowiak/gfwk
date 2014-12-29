/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "draw.h"

#include "cmp_ori.h"
#include "cmp_operations.h"

#include "sc_data.h"

#include "sc_hunter.h"
#include "sc_level.h"
#include "sc_soul.h"
#include "sc_arrow.h"
#include "sc_graph.h"

static void sc_draw_level_tile(struct TilePos tile_pos, char c)
{
	struct WorldPos world_pos;
	struct ScreenPos screen_pos;
	void *bitmap;

	if (c == '#') {
		bitmap = sc_tile;
	}
	else if (c == 's') {
		bitmap = sc_soulbooth;
	}
	else {
		return;
	}

	world_pos = pos_tile_to_world(tile_pos);
	screen_pos = pos_world_to_screen(world_pos);
	draw_bitmap(
		bitmap,
		screen_pos.x + sc_tile_w / 2,
		screen_pos.y + sc_tile_w / 2,
		0.0);
}

void sc_draw_level(struct Level *lvl)
{
	lvl_for_each_tile(lvl, sc_draw_level_tile);
}

void sc_draw_hunter(struct Hunter *hunter)
{
	struct PosRot hunter_pr = cmp_ori_get(&hunter->ori);
	struct WorldPos hunter_wp = { hunter_pr.x, hunter_pr.y - 15.0 };
	struct ScreenPos hunter_sp = pos_world_to_screen(hunter_wp);
	struct WorldPos zero_wp = { 0.0, 0.0 };
	struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);

	cmp_draw(&hunter->ori, &hunter->appr, -zero_sp.x, -zero_sp.y);
	draw_bitmap(sc_bow_bitmap, hunter_sp.x, hunter_sp.y, hunter->aim_angle);
}

void sc_draw_souls(struct SoulArray *souls)
{
	struct WorldPos zero_wp = { 0.0, 0.0 };
	struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);
	int i;

	for (i = 0; i < souls->size; ++i) {
		struct Soul *soul = souls->data + i;
		cmp_draw(&soul->ori, &soul->appr, -zero_sp.x, -zero_sp.y);
	}
}

void sc_draw_souls_dbg(struct SoulArray *souls)
{
	int i;
	ALLEGRO_COLOR color = al_map_rgb_f(1.0f, 0.7f, 0.4f);

	for (i = 0; i < souls->size; ++i) {

		struct Soul *soul = souls->data + i;

		struct PosRot soul_pr = cmp_ori_get(&soul->ori);
		struct WorldPos soul_wp = { soul_pr.x, soul_pr.y };
		struct ScreenPos soul_sp = pos_world_to_screen(soul_wp);

		al_draw_circle(soul_sp.x, soul_sp.y, 4, color, 1);
	}
}

void sc_draw_arrows(struct ArrowArray *aa)
{
	int i;
	struct WorldPos zero_wp = { 0.0, 0.0 };
	struct ScreenPos zero_sp = pos_world_to_screen(zero_wp);
	for (i = 0; i < aa->size; ++i) {
		cmp_draw(&aa->data[i].ori, &aa->data[i].appr, -zero_sp.x, -zero_sp.y);
	}
}

void sc_draw_graph_dbg(struct Graph *lgph)
{
	int i;
	char buf[10];

	for (i = 0; i < lgph->nodes_count; ++i) {

		struct Adj *adj;
		struct TilePos atp = lgph->nodes[i];
		struct WorldPos awp = pos_tile_to_world_ground(atp);
		struct ScreenPos asp = pos_world_to_screen(awp);
		double x1 = asp.x;
		double y1 = asp.y;

		for (adj = lgph->adjacency[i]; adj->neighbor != -1; ++adj) {

			struct TilePos btp = lgph->nodes[adj->neighbor];
			struct WorldPos bwp = pos_tile_to_world_ground(btp);
			struct ScreenPos bsp = pos_world_to_screen(bwp);
			double x2 = bsp.x;
			double y2 = bsp.y;

			ALLEGRO_COLOR color;
			switch (adj->type) {
			case ADJ_WALK:
				color = al_map_rgb_f(0.5f, 1.0f, 0.5f);
				break;
			case ADJ_JUMP:
				color = al_map_rgb_f(0.5f, 0.5f, 1.0f);
				break;
			}

			al_draw_line(x1, y1, x2, y2, color, 1.0);
			al_draw_filled_circle(x1 - 4.0, y1, 5.0, color);
			al_draw_circle(x2 + 4.0, y2, 5.0, color, 1.0);

			sprintf(buf, "%d", i);
			draw_text(sc_debug_font, x1 + 2, y1 - 18, 0, 0, 0, 1, true, buf);
			draw_text(sc_debug_font, x1, y1 - 20, 1, 1, 1, 1, true, buf);

			sprintf(buf, "%d", adj->neighbor);
			draw_text(sc_debug_font, x2 + 2, y2 - 18, 0, 0, 0, 1, true, buf);
			draw_text(sc_debug_font, x2, y2 - 20, 1, 1, 1, 1, true, buf);
		}
	}
}