//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_BBOX_H__
#define GP_BBOX_H__

#include <core/gp_types.h>
#include <core/gp_common.h>

typedef struct gp_bbox {
	gp_coord x;
	gp_coord y;
	gp_size w;
	gp_size h;
} gp_bbox;

static inline int gp_bbox_empty(gp_bbox box)
{
	return box.w == 0 || box.h == 0;
}

static inline gp_bbox gp_bbox_pack(gp_coord x, gp_coord y, gp_coord w, gp_coord h)
{
	gp_bbox box = {
		.x = x,
		.y = y,
		.w = w,
		.h = h,
	};

	return box;
}

static inline gp_bbox gp_bbox_merge(gp_bbox box1, gp_bbox box2)
{
	gp_bbox box = {
		.x = GP_MIN(box1.x, box2.x),
		.y = GP_MIN(box1.y, box2.y),
		.w = GP_MAX(box1.x + box1.w, box2.x + box2.w) - GP_MIN(box1.x, box2.x),
		.h = GP_MAX(box1.y + box1.h, box2.y + box2.h) - GP_MIN(box1.y, box2.y),
	};

	return box;
}

static inline gp_bbox gp_bbox_intersection(gp_bbox box1, gp_bbox box2)
{
	gp_bbox box = {
		.x = GP_MAX(box1.x, box2.x),
		.y = GP_MAX(box1.y, box2.y),
		.w = GP_MIN(box1.x + box1.w, box2.x + box2.w) - GP_MAX(box1.x, box2.x),
		.h = GP_MIN(box1.y + box1.h, box2.y + box2.h) - GP_MAX(box1.y, box2.y),
	};

	return box;
}

static inline int gp_bbox_intersects(gp_bbox box1, gp_bbox box2)
{
	if (box1.x > box2.x + (gp_coord)box2.w)
		return 0;

	if (box2.x > box1.x + (gp_coord)box1.w)
		return 0;

	if (box1.y > box2.y + (gp_coord)box2.h)
		return 0;

	if (box2.y > box1.y + (gp_coord)box1.h)
		return 0;

	return 1;
}

#define GP_BBOX_FMT "[%i, %i] w=%u h=%u"
#define GP_BBOX_PARS(bbox) (bbox).x, (bbox).y, (bbox).w, (bbox).h

#endif /* GP_BBOX_H__ */
