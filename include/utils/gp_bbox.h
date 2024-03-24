//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_bbox.h
 * @brief A bounding box implementation.
 */

#ifndef GP_BBOX_H
#define GP_BBOX_H

#include <core/gp_types.h>
#include <core/gp_common.h>

/**
 * @brief A bounding box.
 */
typedef struct gp_bbox {
	gp_coord x;
	gp_coord y;
	gp_size w;
	gp_size h;
} gp_bbox;

/**
 * @brief Returns true if bounding box is empty.
 *
 * @return Returns true on empty bounding box.
 */
static inline int gp_bbox_empty(gp_bbox box)
{
	return box.w == 0 || box.h == 0;
}

/**
 * @brief Creates a bounding box from coordinates and size.
 *
 * @param x A bounding box left x coordinate.
 * @param y A bounding box top y coordinate.
 * @param w A bounding box width.
 * @param h A bounding box height.
 *
 * @return A gp_bbox initialized from the parameters.
 */
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

/**
 * @brief Merges two bounding boxes.
 *
 * @param box1 First bounding box.
 * @param box2 Second bounding box.
 *
 * @return A bounding box that contains both bounding boxes.
 */
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

/**
 * @brief Calculates a bounding box intersection.
 *
 * @param box1 First bounding box.
 * @param box2 Second bounding box.
 *
 * @return An intersection of two bounding boxes.
 */
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

/**
 * @brief Returns true if bouding boxes intersects.
 *
 * @param box1 First bounding box.
 * @param box2 Second bounding box.
 *
 * @return True if bounding boxes intersects.
 */
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

/**
 * @brief Moves bounding box.
 *
 * @param box A bounding box.
 * @param x An offset to move by in horizontal direction.
 * @param y An offset to move by in vertical direction.
 *
 * @return A bounding box moved by x, y.
 */
static inline gp_bbox gp_bbox_move(gp_bbox box, gp_coord x, gp_coord y)
{
	return gp_bbox_pack(box.x + x, box.y + y, box.w, box.h);
}

/** @brief A bounding box printf format string */
#define GP_BBOX_FMT "[%i, %i] w=%u h=%u"
/** @brief A bounding box printf parameters */
#define GP_BBOX_PARS(bbox) (bbox).x, (bbox).y, (bbox).w, (bbox).h

#endif /* GP_BBOX_H */
