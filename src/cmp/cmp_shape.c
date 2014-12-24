/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "memory.h"

#include "cmp_shape.h"

void cmp_shape_circle_init(struct CmpShape *circle, double r)
{
	circle->type = CMP_SHAPE_CIRCLE;
	circle->body.circle.r = r;
}

void cmp_shape_circle_deinit(struct CmpShape *circle)
{
	(void)circle;
}

struct CmpShape *cmp_shape_circle_create(double r)
{
	struct CmpShape *result = malloc_or_die(sizeof(*result));
	cmp_shape_circle_init(result, r);
	return result;
}

void cmp_shape_segment_init(struct CmpShape *segment, double x1, double y1)
{
	segment->type = CMP_SHAPE_SEGMENT;
	segment->body.segment.x1 = x1;
	segment->body.segment.y1 = y1;
}

void cmp_shape_segment_deinit(struct CmpShape *segment)
{
	(void)segment;
}

struct CmpShape *cmp_shape_segment_create(double x1, double y1)
{
	struct CmpShape *result = malloc_or_die(sizeof(*result));
	cmp_shape_segment_init(result, x1, y1);
	return result;
}

void cmp_shape_free(struct CmpShape *shape)
{
	switch (shape->type) {
	case CMP_SHAPE_CIRCLE:
		cmp_shape_circle_deinit(shape);
		break;

	case CMP_SHAPE_SEGMENT:
		cmp_shape_segment_deinit(shape);
		break;
	}
	free_or_die(shape);
}
