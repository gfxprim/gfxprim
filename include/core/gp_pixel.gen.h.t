@ include header.t
/*
 * Pixel type definitions and functions.
 * Do not include directly, use gp_pixel.h
 *
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2013-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_pixel.gen.h
 * @brief A pixel defintions generated from gen/include/gfxprim_config.py
 *
 * @warning All the actual values here depends on the configuration. If you
 *          change the definitions of pixel types in the gfxprim_config.py
 *          values will be different!
 */

#include <core/gp_clamp.h>

/**
 * @brief List of all pixel types.
 *
 * The type always contains GP_PIXEL_UNKNOWN = 0 and GP_PIXEL_MAX as the last
 * value (also the number of valid types) The types are numbered to use the
 * entire range 0 .. GP_PIXEL_MAX-1. Some pixel types have aliases defined in
 * #gp_pixel_type_alias .
 *
 * You can index the #gp_pixel_types array with a pixel type to get detailed
 * info about a pixel type at a runtime.
 *
 *
 * A pixel definition is a two layered, the top layer defines a packing i.e.
 * #gp_pixel_pack which describes pixel size and how pixels are stored in the
 * pixmap buffer, then pixel type completes the definition by adding how
 * channels are defined in each individual pixel.
 *
 * Some functions need only pixel packing to operate correctly, e.g. if you
 * need to fill in a buffer with a single pixel value, while other need to know
 * the whole pixel type, e.g. filters that operate on individual channel values.
 *
 * Note that two pixels can have the same channels, packing and size but stil
 * differ e.g. RGB888 and BGR888 or same channels, different packing and
 * size, e.g. RGB888 and xRGB8888 or same channels and size but different
 * packing e.g. G1_UB and G1_DB which differ in the direction bits are read in
 * the byte.
 *
 * Also note that on the top of this pixel channels may not be linear and hence
 * may need to be linearized before you can operate on the values. That is not
 * encoded in the pixel type but rather at the gp_pixmap level where you can
 * set a gp_gamma correction with gp_pixmap_gamma_set() independently of the
 * pixel type.
 */
typedef enum gp_pixel_type {
@ for pt in pixeltypes:
	/** @brief Pixel type {{ pt.name }} */
	GP_PIXEL_{{ pt.name }},
@ end
	/** @brief Last valid pixel type has value GP_PIXEL_MAX-1 */
	GP_PIXEL_MAX,
} gp_pixel_type;

@ for pt in pixeltypes:
#define GP_PIXEL_{{ pt.name }} GP_PIXEL_{{ pt.name }}
@ end

@ max_chans = 0
@ for pt in pixeltypes:
@     if pt.chan_cnt > max_chans:
@         max_chans = pt.chan_cnt
@ end
/**
 * @brief Maximal number of channels per all defined pixel types.
 *
 * @warning The value may change if gfxprim_config.py is changed.
 */
#define GP_PIXEL_CHANS_MAX {{ max_chans }}

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
