@ include header.t
/*
 * Pixel channel operations.
 *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2013-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_pixel_channel.gen.h
 * @brief A pixel channel manipulations generated from gen/include/gfxprim_config.py
 *
 * @warning All the actual values here depends on the configuration. If you
 *          change the definitions of pixel types in the gfxprim_config.py
 *          values will be different!
 */

#include <core/gp_clamp.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown():
/**
 * @brief Macro to get raw channel value for {{ pt.name }} pixel.
 */
@         for c in pt.chanslist:
#define GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(p) (GP_GET_BITS({{ c[1] }}, {{ c[2] }}, (p)))
@         end

/*
 * macros to create gp_pixel of pixel type {{ pt.name }} directly from given values.
 * The values MUST be already clipped/converted to relevant value ranges.
 */
#define GP_PIXEL_CREATE_{{ pt.name }}({{ ', '.join(pt.chan_names) }}) (0\
@         for c in pt.chanslist:
	+ (({{ c[0] }}) << {{ c[1] }}) \
@         end
	)

/*
 * macros to do per-channel operations on a pixel.
 */
@ for pt in pixeltypes:
@     if not pt.is_unknown():
#define GP_PIXEL_CHANS_ADD_{{ pt.name }}(pixel, perc) \
	GP_PIXEL_CREATE_{{ pt.name }}( \
@         params = []
@         for c in pt.chanslist:
@             if c.is_alpha:
@                 params.append("GP_PIXEL_GET_" + c.name + "_" + pt.name + "(pixel)")
@             else:
@                 params.append("GP_SAT_ADD(GP_PIXEL_GET_" + c.name + "_" + pt.name + "(pixel), " + c.C_max + " * perc / 100, " + c.C_max + ")")
@         end
		{{ ', \\\n\t\t'.join(params) }} \
	)

@     end
@ end
