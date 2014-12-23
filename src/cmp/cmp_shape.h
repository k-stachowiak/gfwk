/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_SHAPE_H
#define CMP_SHAPE_H

enum CmpShapeType {
	CMP_SHAPE_CIRCLE,
	CMP_SHAPE_SEGMENT
};

struct CmpShapeCircle {
	double r;
};

struct CmpShapeSegment {
	/* x0, y0 are implicitly in (0, 0). */
	double x1, y1;
};

struct CmpShape {
	enum CmpShapeType type;
	union {
		struct CmpShapeCircle circle;
		struct CmpShapeSegment segment;
	} body;
};

void cmp_shape_circle_init(struct CmpShapeCircle *circle, double r);
void cmp_shape_circle_deinit(struct CmpShapeCircle *circle);
struct CmpShape *cmp_shape_circle_create(double r);

void cmp_shape_segment_init(struct CmpShapeSegment *segment, double x1, double y1);
void cmp_shape_segment_deinit(struct CmpShapeSegment *segment);
struct CmpShape *cmp_shape_segment_create(double x1, double y1);

void cmp_shape_free(struct CmpShape *shape);

#endif