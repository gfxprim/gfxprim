@ include source.t
/*
 * Cubic resampling
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <math.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_gamma.h>
#include <core/gp_clamp.h>
#include <core/gp_debug.h>
#include <filters/gp_resize.h>
#include "GP_Cubic.h"

#define MUL 1024

#define MUL_I(a, b) ({ \
	a[0] *= b[0]; \
	a[1] *= b[1]; \
	a[2] *= b[2]; \
	a[3] *= b[3]; \
})

#define SUM_I(a) \
	((a)[0] + (a)[1] + (a)[2] + (a)[3])

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
static int resize_cubic_{{ pt.name }}(const gp_pixmap *src,
	gp_pixmap *dst, gp_progress_cb *callback)
{
@         for c in pt.chanslist:
	int32_t col_{{ c.name }}[src->w];
@         end

	uint32_t i, j;

	GP_DEBUG(1, "Scaling image %ux%u -> %ux%u %2.2f %2.2f",
	            src->w, src->h, dst->w, dst->h,
		    1.00 * dst->w / src->w, 1.00 * dst->h / src->h);

	{@ fetch_gamma_tables(pt, "src") @}

	/* pre-generate x mapping and constants */
	int32_t xmap[dst->w][4];
	int32_t xmap_c[dst->w][4];

	for (i = 0; i < dst->w; i++) {
		float x = (1.00 * i / (dst->w - 1)) * (src->w - 1);

		xmap[i][0] = floor(x - 1);
		xmap[i][1] = x;
		xmap[i][2] = x + 1;
		xmap[i][3] = x + 2;

		xmap_c[i][0] = cubic_int((xmap[i][0] - x) * MUL + 0.5);
		xmap_c[i][1] = cubic_int((xmap[i][1] - x) * MUL + 0.5);
		xmap_c[i][2] = cubic_int((xmap[i][2] - x) * MUL + 0.5);
		xmap_c[i][3] = cubic_int((xmap[i][3] - x) * MUL + 0.5);

		xmap[i][0] = GP_MAX(xmap[i][0], 0);
		xmap[i][2] = GP_MIN(xmap[i][2], (int)src->w - 1);
		xmap[i][3] = GP_MIN(xmap[i][3], (int)src->w - 1);
	}

	/* cubic resampling */
	for (i = 0; i < dst->h; i++) {
		float y = (1.00 * i / (dst->h - 1)) * (src->h - 1);
		int32_t cvy[4];
		int yi[4];

		yi[0] = floor(y - 1);
		yi[1] = y;
		yi[2] = y + 1;
		yi[3] = y + 2;

		cvy[0] = cubic_int((yi[0] - y) * MUL + 0.5);
		cvy[1] = cubic_int((yi[1] - y) * MUL + 0.5);
		cvy[2] = cubic_int((yi[2] - y) * MUL + 0.5);
		cvy[3] = cubic_int((yi[3] - y) * MUL + 0.5);

		yi[0] = GP_MAX(yi[0], 0);
		yi[2] = GP_MIN(yi[2], (int)src->h - 1);
		yi[3] = GP_MIN(yi[3], (int)src->h - 1);	

		/* Generate interpolated row */
		for (j = 0; j < src->w; j++) {
@         for c in pt.chanslist:
			int32_t {{ c.name }}v[4];
@         end
			gp_pixel pix[4];

			pix[0] = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src, j, yi[0]);
			pix[1] = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src, j, yi[1]);
			pix[2] = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src, j, yi[2]);
			pix[3] = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src, j, yi[3]);

@         for c in pt.chanslist:
			{{ c.name }}v[0] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(pix[0]);
			{{ c.name }}v[1] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(pix[1]);
			{{ c.name }}v[2] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(pix[2]);
			{{ c.name }}v[3] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(pix[3]);
@         end

			if (src->gamma) {
@         for c in pt.chanslist:
				{{ c.name }}v[0] = {{ c.name }}_2_LIN[{{ c.name }}v[0]];
				{{ c.name }}v[1] = {{ c.name }}_2_LIN[{{ c.name }}v[1]];
				{{ c.name }}v[2] = {{ c.name }}_2_LIN[{{ c.name }}v[2]];
				{{ c.name }}v[3] = {{ c.name }}_2_LIN[{{ c.name }}v[3]];
@         end
			}

@         for c in pt.chanslist:
			MUL_I({{ c.name }}v, cvy);
@         end

@         for c in pt.chanslist:
			col_{{ c.name }}[j] = SUM_I({{ c.name }}v);
@         end
		}

		/* now interpolate column for new image */
		for (j = 0; j < dst->w; j++) {
@         for c in pt.chanslist:
			int32_t {{ c.name }}v[4];
			int32_t {{ c.name }};
@         end

@         for c in pt.chanslist:
			{{ c.name }}v[0] = col_{{ c.name }}[xmap[j][0]];
			{{ c.name }}v[1] = col_{{ c.name }}[xmap[j][1]];
			{{ c.name }}v[2] = col_{{ c.name }}[xmap[j][2]];
			{{ c.name }}v[3] = col_{{ c.name }}[xmap[j][3]];
@         end

@         for c in pt.chanslist:
			MUL_I({{ c.name }}v, xmap_c[j]);
@         end

@         for c in pt.chanslist:
			{{ c.name }} = (SUM_I({{ c.name }}v) + MUL*MUL/2) / MUL / MUL;
@         end

			if (src->gamma) {
@         for c in pt.chanslist:
				{{ c.name }} = GP_CLAMP_GENERIC({{ c.name }}, 0, {{ 2 ** (c[2] + 2) - 1 }});
@         end
@         for c in pt.chanslist:
				{{ c.name }} = {{ c.name }}_2_GAMMA[{{ c.name }}];
@         end
			} else {
@         for c in pt.chanslist:
				{{ c.name }} = GP_CLAMP_GENERIC({{ c.name }}, 0, {{ 2 ** c[2] - 1 }});
@         end
			}

			gp_pixel pix = GP_PIXEL_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, "(uint8_t)") }});
			gp_putpixel_raw_{{ pt.pixelsize.suffix }}(dst, j, i, pix);
		}

		if (gp_progress_cb_report(callback, i, dst->h, dst->w))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
static int resize_cubic(const gp_pixmap *src, gp_pixmap *dst,
                        gp_progress_cb *callback)
{
	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		return resize_cubic_{{ pt.name }}(src, dst, callback);
	break;
@ end
	default:
		errno = EINVAL;
		return -1;
	}
}

int gp_filter_resize_cubic_int(const gp_pixmap *src, gp_pixmap *dst,
                               gp_progress_cb *callback)
{
	if (src->pixel_type != dst->pixel_type) {
		GP_WARN("The src and dst pixel types must match");
		errno = EINVAL;
		return 1;
	}

	return resize_cubic(src, dst, callback);
}
