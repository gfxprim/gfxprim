@ include header.t
/*
 * Pixel type definitions and functions.
 * Do not include directly, use gp_pixel.h
 *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2013-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_clamp.h>

/*
 * List of all known pixel types
 */
typedef enum gp_pixel_type {
@ for pt in pixeltypes:
	GP_PIXEL_{{ pt.name }},
@ end
	GP_PIXEL_MAX,
} gp_pixel_type;

@ for pt in pixeltypes:
#define GP_PIXEL_{{ pt.name }} GP_PIXEL_{{ pt.name }}
@ end

@ for pt in pixeltypes:
@     if not pt.is_unknown():
/* Automatically generated code for pixel type {{ pt.name }}
 *
 * Size (bpp): {{ pt.pixelpack.size }} ({{ pt.pixelpack.suffix }})
 * Pixel structure: {{ "".join(pt.bits) }}
@         if pt.pixelpack.needs_bit_order():
 * Bit order: {{ pt.pixelpack.bit_order }}
@         end
 * Channels:
@         for c in pt.chanslist:
 *   {{ c[0] }}  offset:{{ c[1] }} size:{{ c[2] }}
@         end
 */

/*
 * macros to get channels of pixel type {{ pt.name }}
 */
@         for c in pt.chanslist:
#define GP_PIXEL_GET_{{ c[0] }}_{{ pt.name }}(p) (GP_GET_BITS({{ c[1] }}, {{ c[2] }}, (p)))
@         end

/*
 * macros to get address and bit-offset of a pixel {{ pt.name }} in a pixmap
 */
#define GP_PIXEL_ADDR_{{ pt.name }}(pixmap, x, y) GP_PIXEL_ADDR_{{ pt.pixelpack.suffix }}(pixmap, x, y)
#define GP_PIXEL_ADDR_OFFSET_{{ pt.name }}(x) GP_PIXEL_ADDR_OFFSET_{{ pt.pixelpack.suffix }}(x)

/*
 * macros to create gp_pixel of pixel type {{ pt.name }} directly from given values.
 * The values MUST be already clipped/converted to relevant value ranges.
 */
#define GP_PIXEL_CREATE_{{ pt.name }}({{ ', '.join(pt.chan_names) }}) (0\
@         for c in pt.chanslist:
	+ (({{ c[0] }}) << {{ c[1] }}) \
@         end
	)

@ end

/*
 * macros for branching on pixel_type
 */

@ for r in [('', ''), ('return ', 'RET_')]:
#define GP_FN_{{ r[1] }}PER_PIXELTYPE(FN_NAME, type, ...)\
	switch (type) { \
@     for pt in pixeltypes:
@         if not pt.is_unknown():
		case GP_PIXEL_{{ pt.name }}:\
			{{ r[0] }}FN_NAME{{'##'}}_{{ pt.name }}(__VA_ARGS__);\
@             if not r[0]:
		break;\
@             end
@     end
		default: GP_ABORT("Invalid PixelType %d", type);\
	}

@ end

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
