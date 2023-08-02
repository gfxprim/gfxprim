@ include source.t
/*
 * Floyd Steinberg dithering RGB888 -> any pixel
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */
#include <string.h>
#include <errno.h>

#include <core/gp_debug.h>
#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_convert.h>
#include "core/gp_clamp.h"
#include <filters/gp_filter.h>
#include <filters/gp_dither.h>

@ def distribute_error(errors, x, y, w, err):
if ({{ x }} + 1 < {{ w }})
	{{ errors }}[{{ y }}%2][{{ x }}+1] += 7 * {{ err }} / 16;

if ({{ x }} > 1)
	{{ errors }}[!({{ y }}%2)][{{ x }}-1] += 3 * {{ err }} / 16;

{{ errors }}[!({{ y }}%2)][{{ x }}] += 5 * {{ err }} / 16;

if ({{ x }} + 1 < {{ w }})
	{{ errors }}[!({{ y }}%2)][{{ x }}+1] += {{ err }} / 16;
@ end
@
@ for pt in pixeltypes:
@     if pt.is_gray() or pt.is_rgb() and not pt.is_alpha():
/*
 * Floyd Steinberg to {{ pt.name }}
 */
static int floyd_steinberg_to_{{ pt.name }}_raw(const gp_pixmap *src,
                                                gp_pixmap *dst,
                                                gp_progress_cb *callback)
{
@         for c in pt.chanslist:
	float errors_{{ c.name }}[2][src->w];
@         end

	GP_DEBUG(1, "Floyd Steinberg %s to %s %ux%u",
	            gp_pixel_type_name(src->pixel_type),
	            gp_pixel_type_name(GP_PIXEL_{{ pt.name }}),
		    src->w, src->h);

	gp_coord x, y;

@         for c in pt.chanslist:
	memset(errors_{{ c.name }}[0], 0, src->w * sizeof(float));
	memset(errors_{{ c.name }}[1], 0, src->w * sizeof(float));
@         end

	for (y = 0; y < (gp_coord)src->h; y++) {
		for (x = 0; x < (gp_coord)src->w; x++) {
			gp_pixel pix;

			pix = gp_getpixel_raw(src, x, y);
			pix = gp_pixel_to_RGB888(pix, src->pixel_type);

@         for c in pt.chanslist:
@             if pt.is_gray():
			float val_{{ c.name }} = GP_PIXEL_GET_R_RGB888(pix) +
			                       GP_PIXEL_GET_G_RGB888(pix) +
			                       GP_PIXEL_GET_B_RGB888(pix);
@             else:
			float val_{{ c.name }} = GP_PIXEL_GET_{{ c.name }}_RGB888(pix);
@             end
			val_{{ c.name }} += errors_{{ c.name }}[y%2][x];

			float err_{{ c.name }} = val_{{ c.name }};
@             if pt.is_gray():
			gp_pixel res_{{ c.name }} = {{ 2 ** c[2] - 1}} * val_{{ c.name }} / (3 * 255);
			err_{{ c.name }} -= res_{{ c.name }} * (3 * 255) / {{ 2 ** c[2] - 1}};
@             else:
			gp_pixel res_{{ c.name }} = {{ 2 ** c[2] - 1}} * val_{{ c.name }} / 255;
			err_{{ c.name }} -= res_{{ c.name }} * 255 / {{ 2 ** c[2] - 1}};
@             end

			{@ distribute_error('errors_' + c.name, 'x', 'y', '(gp_coord)src->w', 'err_' + c.name) @}

			GP_CLAMP_DOWN({{ 'res_' + c.name }}, {{ c.max }});
@         end

@         if pt.is_gray():
			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x, y, res_V);
@         else:
			gp_pixel res = GP_PIXEL_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, 'res_') }});

			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x, y, res);
@         end
		}

@         for c in pt.chanslist:
		memset(errors_{{ c.name }}[y%2], 0, src->w * sizeof(float));
@         end

		if (gp_progress_cb_report(callback, y, src->h, src->w))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
static int floyd_steinberg(const gp_pixmap *src, gp_pixmap *dst,
                           gp_progress_cb *callback)
{
	if (gp_pixel_has_flags(src->pixel_type, GP_PIXEL_IS_PALETTE)) {
		GP_DEBUG(1, "Unsupported source pixel type %s",
		         gp_pixel_type_name(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	switch (dst->pixel_type) {
@ for pt in pixeltypes:
@     if pt.is_gray() or pt.is_rgb() and not pt.is_alpha():
	case GP_PIXEL_{{ pt.name }}:
		return floyd_steinberg_to_{{ pt.name }}_raw(src, dst, callback);
@ end
	default:
		errno = EINVAL;
		return 1;
	}
}

int gp_filter_floyd_steinberg(const gp_pixmap *src, gp_pixmap *dst,
                            gp_progress_cb *callback)
{
	GP_CHECK(src->w <= dst->w);
	GP_CHECK(src->h <= dst->h);

	return floyd_steinberg(src, dst, callback);
}


gp_pixmap *gp_filter_floyd_steinberg_alloc(const gp_pixmap *src,
                                           gp_pixel_type pixel_type,
                                           gp_progress_cb *callback)
{
	gp_pixmap *ret;

	ret = gp_pixmap_alloc(src->w, src->h, pixel_type);

	if (ret == NULL)
		return NULL;

	if (floyd_steinberg(src, ret, callback)) {
		gp_pixmap_free(ret);
		return NULL;
	}

	return ret;
}
