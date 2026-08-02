@ include header.t
/*
 * Copyright (C) 2013-2026 Cyril Hrubis <metan@ucw.cz>
 */
/**
 * @file gp_pixel_alpha.gen.h
 * @brief A pixel alpha channel manipulations generated from gen/include/gfxprim_config.py
 */

#include <core/gp_convert_scale.gen.h>

/**
 * @brief Returns normalized value of the pixel alpha channel.
 *
 * @param pixel A pixel value.
 * @param pixel_type A pixel type for the pixel value.
 *
 * @return A normalized alpha channel i.e. value between 0 and 0xff. Returns 0xff
 *         for pixel types without alpha channel.
 */
static inline gp_pixel gp_get_pixel_norm_alpha(gp_pixel pixel, gp_pixel_type pixel_type)
{
	switch (pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and pt.is_alpha():
	case GP_PIXEL_{{ pt.name }}:
		return GP_SCALE_VAL_{{ pt.chans['A'].size }}_8(GP_PIXEL_GET_A_{{ pt.name }}(pixel));
@ end
	default:
		return 0xff;
	}
}

