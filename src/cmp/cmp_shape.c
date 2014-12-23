/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "memory.h"

#include "cmp_shape.h"

void cmp_shape_circle_init(struct CmpShapeCircle *circle, double r)
{
	circle->r = r;
}

void cmp_shape_circle_deinit(struct CmpShapeCircle *circle)
{
	(void)circle;
}

struct CmpShape *cmp_shape_circle_create(double r)
{
	struct CmpShape *result = malloc_or_die(sizeof(*result));
	result->type = CMP_SHAPE_CIRCLE;
	cmp_shape_circle_init(&result->body.circle, r);
	return result;
}

void cmp_shape_segment_init(struct CmpShapeSegment *segment, double x1, double y1)
{
	segment->x1 = x1;
	segment->y1 = y1;
}

void cmp_shape_segment_deinit(struct CmpShapeSegment *segment)
{
	(void)segment;
}

struct CmpShape *cmp_shape_segment_create(double x1, double y1)
{
	struct CmpShape *result = malloc_or_die(sizeof(*result));
	result->type = CMP_SHAPE_SEGMENT;
	cmp_shape_segment_init(&result->body.segment, x1, y1);
	return result;
}

void cmp_shape_free(struct CmpShape *shape)
{
	switch (shape->type) {
	case CMP_SHAPE_CIRCLE:
		cmp_shape_circle_deinit(&shape->body.circle);
		break;

	case CMP_SHAPE_SEGMENT:
		cmp_shape_segment_deinit(&shape->body.segment);
		break;
	}
	free_or_die(shape);
}