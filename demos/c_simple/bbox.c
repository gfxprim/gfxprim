//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * Just a simple fun with bboxes.
 */

#include <utils/gp_bbox.h>

static void bboxes_intersect(gp_bbox box1, gp_bbox box2)
{
	printf(GP_BBOX_FMT " & " GP_BBOX_FMT " = %i\n",
	       GP_BBOX_PARS(box1), GP_BBOX_PARS(box2),
	       gp_bbox_intersects(box1, box2));
}

int main(void)
{
	gp_bbox bbox = gp_bbox_merge(gp_bbox_pack(0, 0, 1, 1), gp_bbox_pack(1, 1, 1, 1));

	bboxes_intersect(bbox, gp_bbox_pack(0, 0, 1, 1));
	bboxes_intersect(bbox, gp_bbox_pack(0, 3, 1, 1));
	bboxes_intersect(bbox, gp_bbox_pack(2, 0, 1, 1));
	bboxes_intersect(bbox, gp_bbox_pack(-1, -2, 1, 1));

	return 0;
}
