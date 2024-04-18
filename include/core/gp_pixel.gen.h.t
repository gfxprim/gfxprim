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
	/**
	 * @brief Pixel type {{ pt.name }}
	 *
@     if pt.is_unknown():
	 * Returned when pixel matching functions such as gp_pixel_rgb_lookup()
	 * when match wasn't found.
@     else:
	 * {{ ''.join(pt.bits) }}
@     endif
	 */
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
/**
 * @brief A macro to get address of a pixel {{ pt.name }} at x, y in a pixmap.
 *
 * @param pixmap A pixmap.
 * @param x A x coordiate.
 * @param y A y coordiate.
 *
 * @return A pointer to the first byte of the pixel.
 */
#define GP_PIXEL_ADDR_{{ pt.name }}(pixmap, x, y) GP_PIXEL_ADDR_{{ pt.pixelpack.suffix }}(pixmap, x, y)
#define GP_PIXEL_ADDR_OFFSET_{{ pt.name }}(x) GP_PIXEL_ADDR_OFFSET_{{ pt.pixelpack.suffix }}(x)

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
