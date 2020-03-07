// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_convert.h>
#include <core/gp_debug.h>
#include <filters/gp_multi_tone.h>
#include <filters/gp_sepia.h>

#define PIX 3

static void init_sepia_tones(gp_pixel pixels[PIX], gp_pixel_type pixel_type)
{
//	pixels[0] = gp_rgb_to_pixel(56, 33, 10, pixel_type);
//	pixels[1] = gp_rgb_to_pixel(255, 255, 255, pixel_type);
	pixels[0] = gp_rgb_to_pixel(0, 0, 0, pixel_type);
	pixels[1] = gp_rgb_to_pixel(162, 116, 70, pixel_type);
	pixels[2] = gp_rgb_to_pixel(230, 230, 230, pixel_type);
}

int gp_filter_sepia_ex(const gp_pixmap *const src,
                       gp_coord x_src, gp_coord y_src,
                       gp_size w_src, gp_size h_src,
                       gp_pixmap *dst,
                       gp_coord x_dst, gp_coord y_dst,
                       gp_progress_cb *callback)
{
	gp_pixel pixels[PIX];

	init_sepia_tones(pixels, dst->pixel_type);

	return gp_filter_multitone_ex(src, x_src, y_src, w_src, h_src,
	                              dst, x_dst, y_dst, pixels, PIX, callback);
}

gp_pixmap *gp_filter_sepia_ex_alloc(const gp_pixmap *const src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    gp_pixel_type dst_pixel_type,
                                    gp_progress_cb *callback)
{
	gp_pixel pixels[PIX];

	init_sepia_tones(pixels, dst_pixel_type);

	return gp_filter_multitone_ex_alloc(src, x_src, y_src, w_src, h_src,
	                                    dst_pixel_type, pixels, PIX, callback);
}
