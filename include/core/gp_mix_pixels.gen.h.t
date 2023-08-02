@ include header.t
/*
 * Macros to mix two pixels accordingly to percentage.
 *
 * Copyright (C) 2011-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/gp_pixmap.h"
#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_gamma_correction.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown():

/*
 * Mixes two {{ pt.name }} pixels.
 *
 * The percentage is expected as 8 bit unsigned integer [0 .. 255]
 */
#define GP_MIX_PIXELS_LINEAR_{{ pt.name }}(pix1, pix2, perc) ({ \
@         for c in pt.chanslist:
	gp_pixel {{ c[0] }}; \
\
	{{ c[0] }}  = GP_PIXEL_GET_{{ c[0] }}_{{ pt.name }}(pix1) * (perc); \
	{{ c[0] }} += GP_PIXEL_GET_{{ c[0] }}_{{ pt.name }}(pix2) * (255 - (perc)); \
	{{ c[0] }} = ({{ c[0] }} + 128) / 255; \
\
@         end
\
	GP_PIXEL_CREATE_{{ pt.name }}({{ ', '.join(pt.chan_names) }}); \
})

/*
 * Mixes two {{ pt.name }} pixels.
 *
 * The percentage is expected as 8 bit unsigned integer [0 .. 255]
 */
#define GP_MIX_PIXELS_GAMMA_{{ pt.name }}(pix1, pix2, perc) ({ \
@         for c in pt.chanslist:
	gp_pixel {{ c[0] }}; \
\
	{{ c[0] }}  = gp_gamma{{ c[2] }}_to_linear10(GP_PIXEL_GET_{{ c[0] }}_{{ pt.name }}(pix1)) * (perc); \
	{{ c[0] }} += gp_gamma{{ c[2] }}_to_linear10(GP_PIXEL_GET_{{ c[0] }}_{{ pt.name }}(pix2)) * (255 - (perc)); \
	{{ c[0] }} = ({{ c[0] }} + 128) / 255; \
	{{ c[0] }} = gp_linear10_to_gamma{{ c[2] }}({{ c[0] }}); \
\
@         end
\
	GP_PIXEL_CREATE_{{ pt.name }}({{ ", ".join(pt.chan_names) }}); \
})

#define GP_MIX_PIXELS_{{ pt.name }}(pix1, pix2, perc) \
@         if pt.is_rgb():
	GP_MIX_PIXELS_LINEAR_{{ pt.name }}(pix1, pix2, perc)
@         else:
	GP_MIX_PIXELS_LINEAR_{{ pt.name }}(pix1, pix2, perc)
@ end

static inline gp_pixel gp_mix_pixels(gp_pixel pix1, gp_pixel pix2,
                                     uint8_t perc, gp_pixel_type pixel_type)
{
	switch (pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		return GP_MIX_PIXELS_LINEAR_{{ pt.name }}(pix1, pix2, perc);
@ end
	default:
		GP_ABORT("Unknown pixeltype");
	}
}


@ for pt in pixeltypes:
@     if not pt.is_unknown():
static inline void gp_mix_pixel_raw_{{ pt.name }}(gp_pixmap *pixmap,
			gp_coord x, gp_coord y, gp_pixel pixel, uint8_t perc)
{
	gp_pixel pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(pixmap, x, y);
	pix = GP_MIX_PIXELS_{{ pt.name }}(pixel, pix, perc);
	gp_putpixel_raw_{{ pt.pixelpack.suffix }}(pixmap, x, y, pix);
}

@ end

@ for pt in pixeltypes:
@     if not pt.is_unknown():
static inline void gp_mix_pixel_raw_clipped_{{ pt.name }}(gp_pixmap *pixmap,
			gp_coord x, gp_coord y, gp_pixel pixel, uint8_t perc)
{
	if (GP_PIXEL_IS_CLIPPED(pixmap, x, y))
		return;

	gp_mix_pixel_raw_{{ pt.name }}(pixmap, x, y, pixel, perc);
}

@ end

static inline void gp_mix_pixel_raw(gp_pixmap *pixmap, gp_coord x, gp_coord y,
                                    gp_pixel pixel, uint8_t perc)
{
	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
				gp_mix_pixel_raw_{{ pt.name }}(pixmap, x, y, pixel, perc);
	break;
@ end
	default:
		GP_ABORT("Unknown pixeltype");
	}
}

static inline void gp_mix_pixel_raw_clipped(gp_pixmap *pixmap,
                                            gp_coord x, gp_coord y,
                                            gp_pixel pixel, uint8_t perc)
{
	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		gp_mix_pixel_raw_clipped_{{ pt.name }}(pixmap, x, y, pixel, perc);
	break;
@ end
	default:
		GP_ABORT("Unknown pixeltype");
	}
}
