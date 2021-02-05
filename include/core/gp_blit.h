// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2012 Cyril Hrubis    <metan@ucw.cz>
 */

/*

   These blits automatically converts pixel types, that's good (and fast), but
   there is a catch. This works rather well when the number of colors per
   pixel/color channel is increased (the gamma correction is still on TODO).
   However when the number of colors is decreased it's generally better to use
   dithering, which will yield into far better (you can use Floyd Steinberg
   filter for that).

   Also variants without the _raw suffix do honor the rotation flags, that may
   get a little tricky as the flags for rotation are put together but don't
   worry althouth there is some algebra involved the result is quite intuitive.

 */

#ifndef CORE_GP_BLIT_H
#define CORE_GP_BLIT_H

/*
 * Blits rectangle from src defined by x0, y0, x1, y1 (x1, y1 included) to dst
 * starting on x2, y2.
 */
void gp_blit_xyxy(const gp_pixmap *src,
                  gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                  gp_pixmap *dst, gp_coord x2, gp_coord y2);

/*
 * Clipped variant. Could handle destination coordinates outside of the
 * destination rectangle (both possitive and negative). Source larger than
 * destination and so.
 */
void gp_blit_xyxy_clipped(const gp_pixmap *src,
                          gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
                          gp_pixmap *dst, gp_coord x2, gp_coord y2);

/*
 * Blits rectangle from src defined by x0, y0, w0, h0 (uses w0 x h0 pixels) to
 * dst starting on x2, y2.
 */
void gp_blit_xywh(const gp_pixmap *src,
                  gp_coord x0, gp_coord y0, gp_size w0, gp_size h0,
                  gp_pixmap *dst, gp_coord x1, gp_coord y1);

/*
 * Clipped variant. Could handle destination coordinates outside of the
 * destination rectangle (both possitive and negative). Source larger than
 * destination and so.
 */
void gp_blit_xywh_clipped(const gp_pixmap *src,
                          gp_coord x0, gp_coord y0, gp_size w0, gp_size h0,
                          gp_pixmap *dst, gp_coord x1, gp_coord y1);

/* The default is _xywh */
static inline void gp_blit(const gp_pixmap *src,
                           gp_coord x0, gp_coord y0,
                           gp_size w0, gp_size h0,
                           gp_pixmap *dst, gp_coord x1, gp_coord y1)
{
	gp_blit_xywh(src, x0, y0, w0, h0, dst, x1, y1);
}

static inline void gp_blit_clipped(const gp_pixmap *src,
                                   gp_coord x0, gp_coord y0,
                                   gp_size w0, gp_size h0,
                                   gp_pixmap *dst, gp_coord x1, gp_coord y1)
{
	gp_blit_xywh_clipped(src, x0, y0, w0, h0, dst, x1, y1);
}

/*
 * Same as gp_blit_xyxy but doesn't respect rotations. Faster (for now).
 */
void gp_blit_xyxy_raw(const gp_pixmap *src,
                      gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1,
		      gp_pixmap *dst, gp_coord x2, gp_coord y2);

/*
 * Same as gp_blit_xywh but doesn't respect rotations. Faster (for now).
 */
void gp_blit_xywh_raw(const gp_pixmap *src,
                      gp_coord x0, gp_coord y0, gp_size w0, gp_size h0,
		      gp_pixmap *dst, gp_coord x2, gp_coord y2);

/*
 * Same as gp_blit but doesn't respect rotations. Faster (for now).
 */
static inline void gp_blit_raw(const gp_pixmap *src,
                               gp_coord x0, gp_coord y0,
                               gp_size w0, gp_size h0,
                               gp_pixmap *dst, gp_coord x1, gp_coord y1)
{
	gp_blit_xywh_raw(src, x0, y0, w0, h0, dst, x1, y1);
}

#endif /* CORE_GP_BLIT_H */
