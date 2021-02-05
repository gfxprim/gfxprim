// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_RECT_H
#define GP_RECT_H

#include "core/gp_types.h"

/* Rectangle */

void gp_rect_xyxy(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                  gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_rect_xyxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                      gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_rect_xywh(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                  gp_size w, gp_size h, gp_pixel pixel);

void gp_rect_xywh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                      gp_size w, gp_size h, gp_pixel pixel);

/* The xyxy argument set is the default */
static inline void gp_rect(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                           gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_rect_xyxy(pixmap, x0, y0, x1, y1, pixel);
}

static inline void gp_rect_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                               gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_rect_xyxy_raw(pixmap, x0, y0, x1, y1, pixel);
}

/* Filled Rectangle */

void gp_fill_rect_xyxy(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                       gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_fill_rect_xyxy_raw(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                           gp_coord x1, gp_coord y1, gp_pixel pixel);

void gp_fill_rect_xywh(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                       gp_size w, gp_size h, gp_pixel pixel);

void gp_fill_rect_xywh_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y,
	                   gp_size w, gp_size h, gp_pixel pixel);

/* The xyxy argument set is the default */
static inline void gp_fill_rect(gp_pixmap *pixmap, gp_coord x0, gp_coord y0,
                                gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_fill_rect_xyxy(pixmap, x0, y0, x1, y1, pixel);
}

static inline void gp_fill_rect_raw(gp_pixmap *pixmap,
                                    gp_coord x0, gp_coord y0,
                                    gp_coord x1, gp_coord y1, gp_pixel pixel)
{
	gp_fill_rect_xyxy_raw(pixmap, x0, y0, x1, y1, pixel);
}

#endif /* GP_RECT_H */
