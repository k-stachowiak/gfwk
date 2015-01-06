/* Copyright (C) 2015 Krzysztof Stachowiak */

#include "sc_data.h"

#include "sc_booth.h"

void booth_init(struct Booth *booth, long id, struct TilePos tile_pos)
{
	struct WorldPos wp = pos_tile_to_world_mid(tile_pos);
	booth->id = id;
	cmp_appr_static_sprite_init(&booth->appr, sc_soulbooth);
	cmp_pain_init(&booth->pain, PT_BOOTH);
	cmp_ori_init(&booth->ori, wp.x, wp.y, 0.0);
	cmp_shape_circle_init(&booth->shape, 25.0);
}

void booth_deinit(struct Booth *booth)
{
	cmp_appr_deinit(&booth->appr);
	cmp_pain_deinit(&booth->pain);
	cmp_ori_deinit(&booth->ori);
	cmp_shape_deinit(&booth->shape);
}