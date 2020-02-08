@ include source.t
/*
 * Hilbert Peano dithering RGB888 -> any pixel
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include <core/gp_debug.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_convert.h>
#include <filters/gp_hilbert_curve.h>
#include <filters/gp_filter.h>
#include <filters/gp_dither.h>

/*
 * Returns closest greater square of two, used to determine the curve size.
 */
static unsigned int count_bits(unsigned int n)
{
	unsigned int i = 0, s = n;

	do {
		n>>=1;
		i++;
	} while (n);

	i--;

	return (i + (s != (1U<<i)));
}

@ for pt in pixeltypes:
@     if pt.is_gray() or pt.is_rgb() and not pt.is_alpha():
/*
 * Hilbert Peano to {{ pt.name }}
 */
static int hilbert_peano_to_{{ pt.name }}_raw(const gp_pixmap *src,
                                              gp_pixmap *dst,
                                              gp_progress_cb *callback)
{
	gp_curve_state state;
	unsigned int n;

	n = GP_MAX(count_bits(src->w), count_bits(src->h));

	GP_DEBUG(1, "Hilbert Peano dithering %ux%u -> n = %u", src->w, src->h, n);

	gp_hilbert_curve_init(&state, n);

	/* processed pixels counter */
	unsigned int cnt = 0;

	/* error counters */
@         for c in pt.chanslist:
	int err_{{ c[0] }} = 0;
@         end

	while (gp_hilbert_curve_continues(&state)) {
		if (state.x < src->w && state.y < src->h) {
			gp_pixel pix;

			pix = gp_getpixel_raw(src, state.x, state.y);
			pix = gp_pixel_to_RGB888(pix, src->pixel_type);

@         for c in pt.chanslist:
@             if pt.is_gray():
			int pix_{{ c[0] }} = GP_PIXEL_GET_R_RGB888(pix) +
			                     GP_PIXEL_GET_G_RGB888(pix) +
			                     GP_PIXEL_GET_B_RGB888(pix);
@             else:
			int pix_{{ c[0] }} = GP_PIXEL_GET_{{ c[0] }}_RGB888(pix);
@             end
			pix_{{ c[0] }} += err_{{ c[0] }};

@             if pt.is_gray():
			int res_{{ c[0] }} = ({{ 2 ** c[2] - 1}} * pix_{{ c[0] }} + 382) / {{ 3 * 255 }};
			err_{{ c[0] }} = pix_{{ c[0] }} - {{ 3 * 255 }} * res_{{ c[0] }} / {{ 2 ** c[2] - 1 }};
@             else:
			int res_{{ c[0] }} = ({{ 2 ** c[2] - 1}} * pix_{{ c[0] }} + 127) / 255;
			err_{{ c[0] }} = pix_{{ c[0] }} - 255 * res_{{ c[0] }} / {{ 2 ** c[2] - 1 }};
@         end

@         if pt.is_gray():
			gp_putpixel_raw_{{ pt.pixelsize.suffix }}(dst, state.x, state.y, res_V);
@         else:
			gp_pixel res = GP_PIXEL_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, 'res_') }});

			gp_putpixel_raw_{{ pt.pixelsize.suffix }}(dst, state.x, state.y, res);
@         end
			cnt++;

			if (gp_progress_cb_report(callback, cnt/src->h, src->w, src->h))
				return 1;

			/* We are done, exit */
			if (cnt == src->w * src->h - 1) {
				gp_progress_cb_done(callback);
				return 0;
			}
		} else {
@         for c in pt.chanslist:
			err_{{ c[0] }} = 0;
@         end
		}

		gp_hilbert_curve_next(&state);
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
static int hilbert_peano(const gp_pixmap *src, gp_pixmap *dst,
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
		return hilbert_peano_to_{{ pt.name }}_raw(src, dst, callback);
@ end
	default:
		errno = EINVAL;
		return 1;
	}
}

int gp_filter_hilbert_peano(const gp_pixmap *src, gp_pixmap *dst,
                            gp_progress_cb *callback)
{
	GP_CHECK(src->w <= dst->w);
	GP_CHECK(src->h <= dst->h);

	return hilbert_peano(src, dst, callback);
}

gp_pixmap *gp_filter_hilbert_peano_alloc(const gp_pixmap *src,
                                         gp_pixel_type pixel_type,
                                         gp_progress_cb *callback)
{
	gp_pixmap *ret;

	ret = gp_pixmap_alloc(src->w, src->h, pixel_type);

	if (ret == NULL)
		return NULL;

	if (hilbert_peano(src, ret, callback)) {
		gp_pixmap_free(ret);
		return NULL;
	}

	return ret;
}
