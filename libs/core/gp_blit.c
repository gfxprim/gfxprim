// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2013 Cyril Hrubis    <metan@ucw.cz>
 */

#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include "core/gp_pixmap.h"
#include <core/gp_convert.h>
#include <core/gp_debug.h>
#include <core/gp_blit.h>

/* Generated functions */
void gp_blit_xyxy_raw_fast(const gp_pixmap *src,
                           gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                           gp_pixmap *dst, gp_coord x2, gp_coord y2);

void gp_blit_xyxy_fast(const gp_pixmap *src,
                       gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                       gp_pixmap *dst, gp_coord x2, gp_coord y2);

void gp_blit_xyxy(const gp_pixmap *src,
                  gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                  gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	/* Normalize source rectangle */
	if (x1 < x0)
		GP_SWAP(x0, x1);

	if (y1 < y0)
		GP_SWAP(y0, y1);

	/* All coordinates are inside of src the pixmap */
	GP_CHECK(x0 < (gp_coord) gp_pixmap_w(src));
	GP_CHECK(y0 < (gp_coord) gp_pixmap_h(src));
	GP_CHECK(x1 < (gp_coord) gp_pixmap_w(src));
	GP_CHECK(y1 < (gp_coord) gp_pixmap_h(src));

	/* Destination is big enough */
	GP_CHECK(x2 + (x1 - x0) < (gp_coord) gp_pixmap_w(dst));
	GP_CHECK(y2 + (y1 - y0) < (gp_coord) gp_pixmap_h(dst));

	gp_blit_xyxy_fast(src, x0, y0, x1, y1, dst, x2, y2);
}

void gp_blit_xyxy_clipped(const gp_pixmap *src,
                          gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                          gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	/* Normalize source rectangle */
	if (x1 < x0)
		GP_SWAP(x0, x1);

	if (y1 < y0)
		GP_SWAP(y0, y1);

	/*
	 * Handle all cases where at least one of dest coordinates are out of
	 * the dest in positive direction -> src is out of dst completly.
	 */
	if (x2 >= (gp_coord) gp_pixmap_w(dst) ||
	    y2 >= (gp_coord) gp_pixmap_h(dst))
		return;

	/*
	 * The coordinates in dest are negative.
	 *
	 * We need to clip the source upper left corner accordingly.
	 *
	 * Notice that x2 and y2 are inside the dst rectangle now.
	 * (>= 0 and < w, < h)
	 */
	if (x2 < 0) {
		x0 -= x2;
		x2 = 0;
	}

	if (y2 < 0) {
		y0 -= y2;
		y2 = 0;
	}

	/* Make sure souce coordinates are inside of the src */
	x0 = GP_MAX(x0, 0);
	y0 = GP_MAX(y0, 0);
	x1 = GP_MIN(x1, (gp_coord) gp_pixmap_w(src) - 1);
	y1 = GP_MIN(y1, (gp_coord) gp_pixmap_h(src) - 1);

	/* And source rectangle fits inside of the destination */
	gp_coord src_w = x1 - x0 + 1;
	gp_coord src_h = y1 - y0 + 1;

	gp_coord dst_w = gp_pixmap_w(dst) - x2;
	gp_coord dst_h = gp_pixmap_h(dst) - y2;

	GP_DEBUG(2, "Blitting %ix%i, available %ix%i",
	         src_w, src_h, dst_w, dst_h);

	if (src_w > dst_w)
		x1 -= src_w - dst_w;

	if (src_h > dst_h)
		y1 -= src_h - dst_h;

	GP_DEBUG(2, "Blitting %ix%i->%ix%i in %ux%u to %ix%i in %ux%u",
	         x0, y0, x1, y1, gp_pixmap_w(src), gp_pixmap_h(src),
	         x2, y2, gp_pixmap_w(dst), gp_pixmap_h(dst));

	gp_blit_xyxy_fast(src, x0, y0, x1, y1, dst, x2, y2);
}

void gp_blit_xywh(const gp_pixmap *src,
                  gp_coord x0, gp_coord y0, gp_size w0, gp_size h0,
                  gp_pixmap *dst, gp_coord x1, gp_coord y1)
{
	if (w0 == 0 || h0 == 0)
		return;

	gp_blit_xyxy(src, x0, y0, x0 + w0 - 1, y0 + h0 - 1, dst, x1, y1);
}

void gp_blit_xywh_clipped(const gp_pixmap *src,
                          gp_coord x0, gp_coord y0, gp_size w0, gp_size h0,
                          gp_pixmap *dst, gp_coord x1, gp_coord y1)
{
	if (w0 == 0 || h0 == 0)
		return;

	gp_blit_xyxy_clipped(src, x0, y0, x0+w0-1, y0+h0-1, dst, x1, y1);
}

void gp_blit_xyxy_raw(const gp_pixmap *src,
                      gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                      gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	/* Normalize source rectangle */
	if (x1 < x0)
		GP_SWAP(x0, x1);

	if (y1 < y0)
		GP_SWAP(y0, y1);

	/* All coordinates are inside of src the pixmap */
	GP_CHECK(x0 < (gp_coord)src->w);
	GP_CHECK(y0 < (gp_coord)src->h);
	GP_CHECK(x1 < (gp_coord)src->w);
	GP_CHECK(y1 < (gp_coord)src->h);

	/* Destination is big enough */
	GP_CHECK(x2 + (x1 - x0) < (gp_coord)dst->w);
	GP_CHECK(y2 + (y1 - y0) < (gp_coord)dst->h);

	gp_blit_xyxy_raw_fast(src, x0, y0, x1, y1, dst, x2, y2);
}

void gp_blit_xywh_raw(const gp_pixmap *src,
                     gp_coord x0, gp_coord y0, gp_size w0, gp_size h0,
                     gp_pixmap *dst, gp_coord x2, gp_coord y2)
{
	if (w0 == 0 || h0 == 0)
		return;

	gp_blit_xyxy_raw(src, x0, y0, x0+w0-1, y0+h0-1, dst, x2, y2);
}
