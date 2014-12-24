/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "array.h"

#include "sc_data.h"

#include "sc_arrow.h"
#include "sc_hunter.h"

void sc_shoot_arrow(void)
{
	struct Arrow arrow;
	struct PosRot hunter_pr = cmp_ori_get(&hunter.ori);
	struct WorldPos hunter_wp = { hunter_pr.x, hunter_pr.y };

	arrow_init(
		&arrow,
		++sc_entity_id,
		hunter_wp.x,
		hunter_wp.y - 15.0,
		hunter.aim_angle);

	ARRAY_APPEND(arrows, arrow);
}