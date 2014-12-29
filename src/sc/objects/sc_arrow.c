/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <math.h>

#include "sc_data.h"

#include "sc_arrow.h"

void arrow_init(struct Arrow *arrow, long id, double x, double y, double angle)
{
    static double vel = 600.0;
	arrow->id = id;
	cmp_drv_ballistic_init(&arrow->drv, true, cos(angle) * vel, sin(angle) * vel);
	cmp_appr_static_sprite_init(&arrow->appr, sc_arrow_bitmap);
	cmp_pain_init(&arrow->pain, PT_ARROW);
	cmp_ori_init(&arrow->ori, x, y, angle);
	cmp_shape_segment_init(&arrow->shape, 25.0, 0.0);
}

void arrow_deinit(struct Arrow *arrow)
{
	cmp_drv_deinit(&arrow->drv);
	cmp_appr_deinit(&arrow->appr);
    cmp_pain_deinit(&arrow->pain);
	cmp_ori_deinit(&arrow->ori);
	cmp_shape_segment_deinit(&arrow->shape);
}
