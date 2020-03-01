@ include source.t
/*
 * Gaussian Noise
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_temp_alloc.h>
#include <core/gp_clamp.h>
#include <core/gp_debug.h>

#include <filters/gp_rand.h>
#include <filters/gp_gaussian_noise.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
static int gaussian_noise_add_{{ pt.name }}(const gp_pixmap *src,
                                gp_coord x_src, gp_coord y_src,
                                gp_size w_src, gp_size h_src,
                                gp_pixmap *dst,
                                gp_coord x_dst, gp_coord y_dst,
                                float sigma, float mu,
                                gp_progress_cb *callback)
{
	GP_DEBUG(1, "Additive Gaussian noise filter %ux%u sigma=%f mu=%f",
	         w_src, h_src, sigma, mu);

@         for c in pt.chanslist:
	int sigma_{{ c.name }} = {{ c.max }} * sigma;
	int mu_{{ c.name }} = {{ c.max }} * mu;
@         end

	unsigned int size = w_src + w_src%2;

	/* Create temporary buffers */
	gp_temp_alloc_create(temp, sizeof(int) * size * {{ len(pt.chanslist) }});

@         for c in pt.chanslist:
	int *{{ c.name }} = gp_temp_alloc_get(temp, size * sizeof(int));
@         end

	/* Apply the additive noise filter */
	unsigned int x, y;

	for (y = 0; y < h_src; y++) {
@         for c in pt.chanslist:
		gp_norm_int({{ c.name }}, size, sigma_{{ c.name }}, mu_{{ c.name }});
@         end

		for (x = 0; x < w_src; x++) {
			gp_pixel pix = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src, x + x_src, y + y_src);

@         for c in pt.chanslist:
			{{ c.name }}[x] += GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(pix);
			{{ c.name }}[x] = GP_CLAMP({{ c.name }}[x], 0, {{ c.max }});
@         end

			pix = GP_PIXEL_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, '', '[x]') }});
			gp_putpixel_raw_{{ pt.pixelsize.suffix }}(dst, x + x_dst, y + y_dst, pix);
		}

		if (gp_progress_cb_report(callback, y, h_src, w_src)) {
			gp_temp_alloc_free(temp);
			errno = ECANCELED;
			return 1;
		}
	}

	gp_temp_alloc_free(temp);
	gp_progress_cb_done(callback);

	return 0;
}

@ end
@
int gp_filter_gaussian_noise_add_raw(const gp_pixmap *src,
                                     gp_coord x_src, gp_coord y_src,
                                     gp_size w_src, gp_size h_src,
                                     gp_pixmap *dst,
                                     gp_coord x_dst, gp_coord y_dst,
                                     float sigma, float mu,
                                     gp_progress_cb *callback)
{
	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		return gaussian_noise_add_{{ pt.name }}(src, x_src,
				y_src, w_src, h_src, dst, x_dst, y_dst,
				sigma, mu, callback);
	break;
@ end
	default:
		errno = EINVAL;
		return -1;
	}
}

int gp_filter_gaussian_noise_add_ex(const gp_pixmap *src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    gp_pixmap *dst,
                                    gp_coord x_dst, gp_coord y_dst,
                                    float sigma, float mu,
                                    gp_progress_cb *callback)
{
	GP_CHECK(src->pixel_type == dst->pixel_type);

	/* Check that destination is large enough */
	GP_CHECK(x_dst + (gp_coord)w_src <= (gp_coord)dst->w);
	GP_CHECK(y_dst + (gp_coord)h_src <= (gp_coord)dst->h);

	/* Source is large enough */
	GP_CHECK(x_src + w_src <= src->w);
	GP_CHECK(y_src + h_src <= src->h);

	return gp_filter_gaussian_noise_add_raw(src, x_src, y_src, w_src, h_src,
	                                        dst, x_dst, y_dst,
	                                        sigma, mu, callback);
}

gp_pixmap *gp_filter_gaussian_noise_add_ex_alloc(const gp_pixmap *src,
                                                 gp_coord x_src, gp_coord y_src,
                                                 gp_size w_src, gp_size h_src,
                                                 float sigma, float mu,
                                                 gp_progress_cb *callback)
{
	int ret, err;

	gp_pixmap *dst = gp_pixmap_alloc(w_src, h_src, src->pixel_type);

	if (dst == NULL)
		return NULL;

	ret = gp_filter_gaussian_noise_add_raw(src, x_src, y_src, w_src, h_src,
	                                      dst, 0, 0, sigma, mu, callback);

	if (ret) {
		err = errno;
		gp_pixmap_free(dst);
		errno = err;
		return NULL;
	}

	return dst;
}
