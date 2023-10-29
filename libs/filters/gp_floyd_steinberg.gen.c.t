@ include source.t
/*
 * Floyd Steinberg dithering -> any pixel
 *
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */
#include <string.h>
#include <errno.h>

#include <core/gp_debug.h>
#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_convert.h>
#include <core/gp_clamp.h>
#include <filters/gp_filter.h>
#include <filters/gp_dither.h>

@ def distribute_error_fs(c, x, y, err):
errors_{{ c.name }}[{{ y }}%2][{{ x }}+2] += 7 * {{ err }};

errors_{{ c.name }}[!({{ y }}%2)][{{ x }}] += 3 * {{ err }};

errors_{{ c.name }}[!({{ y }}%2)][{{ x }}+1] += 5 * {{ err }};

errors_{{ c.name }}[!({{ y }}%2)][{{ x }}+2] += {{ err }};
@ end
@
@ def get_error(c, x, y):
errors_{{ c.name }}[{{ y }} % 2][{{ x }} + 1] / 16
@ end
@
@ def clear_errors(pt, w, y):
@     for c in pt.chanslist:
memset(errors_{{ c.name }}[{{ y }} % 2], 0, (2 + {{ w }}) * sizeof(uint32_t));
@     end
@ end
@
@ def def_errors(pt, w):
@     for c in pt.chanslist:
uint32_t errors_{{ c.name }}[2][{{ w }} + 2];
@     end

{@ clear_errors(pt, w, 0) @}
{@ clear_errors(pt, w, 1) @}
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
	gp_coord x, y;

	{@ def_errors(pt, 'src->w') @}

	GP_DEBUG(1, "Floyd Steinberg %s to %s %ux%u",
	            gp_pixel_type_name(src->pixel_type),
	            gp_pixel_type_name(GP_PIXEL_{{ pt.name }}),
		    src->w, src->h);

	for (y = 0; y < (gp_coord)src->h; y++) {
		for (x = 0; x < (gp_coord)src->w; x++) {
			gp_pixel pix;

			pix = gp_getpixel_raw(src, x, y);
@         if pt.is_rgb():
			pix = gp_pixel_to_RGB888(pix, src->pixel_type);
@         end

@         for c in pt.chanslist:
@             if pt.is_gray():
			uint32_t val_{{ c.name }} = gp_pixel_to_G8(pix, src->pixel_type);
@             else:
			uint32_t val_{{ c.name }} = GP_PIXEL_GET_{{ c.name }}_RGB888(pix);
@             end
			val_{{ c.name }} += {@ get_error(c, 'x', 'y') @};

			uint32_t err_{{ c.name }} = val_{{ c.name }};

			gp_pixel res_{{ c.name }} = {{ c.max }} * val_{{ c.name }} / 255;
			err_{{ c.name }} -= res_{{ c.name }} * 255 / {{ c.max }};

			{@ distribute_error_fs(c, 'x', 'y', 'err_' + c.name) @}

			GP_CLAMP_DOWN({{ 'res_' + c.name }}, {{ c.max }});
@         end

@         if pt.is_gray():
			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x, y, res_V);
@         else:
			gp_pixel res = GP_PIXEL_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, 'res_') }});

			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x, y, res);
@         end
		}

		{@ clear_errors(pt, 'src->w', 'y') @}

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

	if (!ret)
		return NULL;

	if (floyd_steinberg(src, ret, callback)) {
		gp_pixmap_free(ret);
		return NULL;
	}

	return ret;
}
