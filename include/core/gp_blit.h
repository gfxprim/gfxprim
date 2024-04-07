// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2024 Cyril Hrubis    <metan@ucw.cz>
 */

/**
 * @file gp_blit.h
 * @brief Pixmap blits.
 */
#ifndef CORE_GP_BLIT_H
#define CORE_GP_BLIT_H

/**
 * @brief Blits a rectangle from src into a dst.
 * @ingroup gfx
 *
 * This blit automatically converts between different #gp_pixel_type, however
 * they operate on a single pixel value at a time, if you are converting from a
 * higher bit depths the results are suboptimal, expecially conversions to 1bpp
 * does not end up as a recognizable image. In these cases an error
 * distribution dithering #gp_dither_type should be used instead.
 *
 * Blits rectangle from src defined by x0, y0, x1, y1 (x1, y1 included) to dst
 * starting on x2, y2.
 *
 * @param src A source pixmap.
 * @param x0 A left rectangle corner coordinate in src.
 * @param y0 A top rectangle corner coordinate in src.
 * @param x1 A right rectangle corner coordinate in src.
 * @param y1 A bottom rectangle corner coordinate in src.
 * @param dst A destination pixmap.
 * @param x2 A left rectangle corner coordinate in dst.
 * @param y2 A top rectangle corner coordinate in dst.
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

/**
 * @brief Blits a rectangle from src into a dst.
 * @ingroup gfx
 *
 * This blit automatically converts between different #gp_pixel_type, however
 * they operate on a single pixel value at a time, if you are converting from a
 * higher bit depths the results are suboptimal, expecially conversions to 1bpp
 * does not end up as a recognizable image. In these cases an error
 * distribution dithering #gp_dither_type should be used instead.
 *
 * Blits rectangle from src defined by x0, y0, x1, y1 (x1, y1 included) to dst
 * starting on x2, y2.
 *
 * @param src A source pixmap.
 * @param x0 A left rectangle corner coordinate in src.
 * @param y0 A top rectangle corner coordinate in src.
 * @param w0 A rectangle width.
 * @param h0 A rectangle height.
 * @param dst A destination pixmap.
 * @param x1 A left rectangle corner coordinate in dst.
 * @param y1 A top rectangle corner coordinate in dst.
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

/**
 * @brief Blits a rectangle from src into a dst.
 * @ingroup gfx
 *
 * An alias for gp_blit_xywh().
 */
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
