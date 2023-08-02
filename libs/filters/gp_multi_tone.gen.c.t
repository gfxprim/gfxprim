@ include source.t
/*
 * Generic Point filer
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include "core/gp_pixmap.h"
#include <core/gp_get_put_pixel.h>
#include <core/gp_temp_alloc.h>
#include <core/gp_mix_pixels.h>
#include <core/gp_debug.h>

#include <filters/gp_multi_tone.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
static void init_table_{{ pt.name }}(gp_pixel table[],
                                     gp_size table_size,
                                     gp_pixel pixels[],
                                     gp_size pixels_size)
{
	unsigned int i;
	unsigned int p = 0;
	float perc;
	float step = 1.00 * table_size / (pixels_size - 1);

	GP_DEBUG(2, "Preparing pixel table %u steps %u pixels, step %.2f",
	         table_size, pixels_size, step);

	for (i = 0; i < table_size; i++) {
		p = 1.00 * i / step;
		perc = i+1;

		while (perc > step)
			perc -= step;

		perc = perc / step;

		table[i] = GP_MIX_PIXELS_{{ pt.name }}(pixels[p+1], pixels[p], 255 * perc);
//		printf("p = %u i = %u PERC %.2f\n", p, i, perc);
//		gp_pixel_print(table[i], GP_PIXEL_{{ pt.name }});
	}
}

@ end
@
static void init_table(gp_pixel_type type,
                       gp_pixel table[], gp_size table_size,
                       gp_pixel pixels[], gp_size pixels_size)
{
	switch (type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		init_table_{{ pt.name }}(table, table_size,
		                         pixels, pixels_size);
	break;
@ end
	default:
		GP_BUG("Should not be reached");
	break;
	}
}

#include <assert.h>

@ for pt in pixeltypes:
@     if pt.is_gray():
static int multitone_{{ pt.name }}(const gp_pixmap *const src,
                                   gp_coord x_src, gp_coord y_src,
                                   gp_size w_src, gp_size h_src,
                                   gp_pixmap *dst,
                                   gp_coord x_dst, gp_coord y_dst,
                                   gp_pixel pixels[], gp_size pixels_size,
                                   gp_progress_cb *callback)
{
@         size = pt.chanslist[0].max + 1
	gp_temp_alloc_create(tmp, {{ size }} * sizeof(gp_pixel));
	gp_pixel *table = gp_temp_alloc_get(tmp, {{ size }} * sizeof(gp_pixel));

	GP_DEBUG(1, "Duotone filter %ux%u {{ pt.name }} -> %s",
	         w_src, h_src, gp_pixel_type_name(dst->pixel_type));

	init_table(dst->pixel_type, table, {{ size }}, pixels, pixels_size);

	unsigned int x, y;

	for (y = 0; y < h_src; y++) {
		for (x = 0; x < w_src; x++) {
			unsigned int src_x = x_src + x;
			unsigned int src_y = y_src + y;
			unsigned int dst_x = x_dst + x;
			unsigned int dst_y = y_dst + y;

			gp_pixel pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(src, src_x, src_y);

			pix = table[pix];

			gp_putpixel_raw(dst, dst_x, dst_y, pix);
		}

		if (gp_progress_cb_report(callback, y, h_src, w_src)) {
			gp_temp_alloc_free(tmp);
			errno = ECANCELED;
			return 1;
		}
	}

	gp_temp_alloc_free(tmp);
	gp_progress_cb_done(callback);

	return 0;
}

@ end
@
int gp_filter_multitone_ex(const gp_pixmap *const src,
                           gp_coord x_src, gp_coord y_src,
                           gp_size w_src, gp_size h_src,
                           gp_pixmap *dst,
                           gp_coord x_dst, gp_coord y_dst,
                           gp_pixel pixels[], gp_size pixels_size,
                           gp_progress_cb *callback)
{
	//CHECK DST IS NOT PALETTE PixelHasFlags

	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if pt.is_gray():
	case GP_PIXEL_{{ pt.name }}:
		return multitone_{{ pt.name }}(src, x_src, y_src,
		                               w_src, h_src, dst,
		                               x_dst, y_dst,
		                               pixels, pixels_size,
		                               callback);
	break;
@ end
	default:
		errno = EINVAL;
		return -1;
	}
}

gp_pixmap *gp_filter_multitone_ex_alloc(const gp_pixmap *const src,
                                        gp_coord x_src, gp_coord y_src,
                                        gp_size w_src, gp_size h_src,
                                        gp_pixel_type dst_pixel_type,
                                        gp_pixel pixels[], gp_size pixels_size,
                                        gp_progress_cb *callback)
{
	gp_pixmap *res;
	int err;

	res = gp_pixmap_alloc(w_src, h_src, dst_pixel_type);

	if (!res) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	if (gp_filter_multitone_ex(src, x_src, y_src, w_src, h_src, res, 0, 0,
	                         pixels, pixels_size, callback)) {
		err = errno;
		gp_pixmap_free(res);
		errno = err;
		return NULL;
	}

	return res;
}
