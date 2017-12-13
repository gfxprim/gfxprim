@ include header.t
/*
 * Macros to mix two pixels. The source must have alpha channel.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_GammaCorrection.h"
#include "core/GP_Pixel.h"

//TODO: Fix blit where both source and destination have alpha channel

@ for src in pixeltypes:
@     if not src.is_unknown() and not src.is_palette():
@         for dst in pixeltypes:
@             if not dst.is_unknown() and not dst.is_palette():
@                 if src.is_alpha():

static inline gp_pixel gp_mix_pixels_{{ src.name }}_{{ dst.name }}(gp_pixel src, gp_pixel dst)
{
	/* Extract the alpha channel */
	unsigned int alpha = GP_PIXEL_GET_A_{{ src.name }}(src);

	/* Convert the pixel to RGB888, mix the values */
	gp_pixel src_rgb = 0, dst_rgb = 0, res = 0;

	GP_PIXEL_{{ src.name }}_TO_RGB888(src, src_rgb);
	GP_PIXEL_{{ dst.name }}_TO_RGB888(dst, dst_rgb);

	int sr, sg, sb;
	int dr, dg, db;

	sr = GP_PIXEL_GET_R_RGB888(src_rgb);
	sg = GP_PIXEL_GET_G_RGB888(src_rgb);
	sb = GP_PIXEL_GET_B_RGB888(src_rgb);

	dr = GP_PIXEL_GET_R_RGB888(dst_rgb);
	dg = GP_PIXEL_GET_G_RGB888(dst_rgb);
	db = GP_PIXEL_GET_B_RGB888(dst_rgb);

@                     a_max = 2 ** src.chans['A'][2] - 1

	dr = (dr * ({{ a_max }} - alpha) + sr * alpha + {{ a_max // 2 }}) / {{ a_max }};
	dg = (dg * ({{ a_max }} - alpha) + sg * alpha + {{ a_max // 2 }}) / {{ a_max }};
	db = (db * ({{ a_max }} - alpha) + sb * alpha + {{ a_max // 2 }}) / {{ a_max }};

	dst_rgb = GP_PIXEL_CREATE_RGB888(dr, dg, db);

	GP_PIXEL_RGB888_TO_{{ dst.name }}(dst_rgb, res);

	return res;
}

