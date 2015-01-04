/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_SHAPE_H
#define CMP_SHAPE_H

/* Circle shape.
 * =============
 */

struct CmpShapeCircle {
	double r;
};

/* Segment shape.
 * ==============
 */
struct CmpShapeSegment {
	/* x0, y0 are implicitly in (0, 0). So technically it's a vector. */
	double x1, y1;
};

/* Root structure.
* ===============
*/

enum CmpShapeType {
	CMP_SHAPE_CIRCLE,
	CMP_SHAPE_SEGMENT
};

struct CmpShape {
	enum CmpShapeType type;
	union {
		struct CmpShapeCircle circle;
		struct CmpShapeSegment segment;
	} body;
};

void cmp_shape_circle_init(struct CmpShape *circle, double r);
void cmp_shape_segment_init(struct CmpShape *segment, double x1, double y1);
void cmp_shape_deinit(struct CmpShape *shape);

#endif