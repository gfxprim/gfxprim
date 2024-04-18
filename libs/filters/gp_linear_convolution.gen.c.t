@ include source.t
/*
 * Linear Convolution
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_temp_alloc.h>
#include <core/gp_clamp.h>
#include <core/gp_debug.h>

#include <filters/gp_linear.h>

#define MUL 1024

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():

static int h_lin_conv_{{ pt.name }}(const gp_pixmap *src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    gp_pixmap *dst,
                                    gp_coord x_dst, gp_coord y_dst,
				    float kernel[], uint32_t kw, float kern_div,
				    gp_progress_cb *callback)
{
	gp_coord x, y;
	uint32_t i;
	int ikernel[kw], ikern_div;
	uint32_t size = w_src + kw - 1;

	/* Fetch gamma tables */
	{@ fetch_gamma_lin(pt, 'src') @}
	{@ fetch_gamma_enc(pt, 'dst') @}

	/* Fetch maximal values for linearized channels */
	{@ fetch_chan_lin_max(pt, 'src') @}

	ikern_div = 0;
	(void) kern_div;

	for (i = 0; i < kw; i++) {
		ikernel[i] = kernel[i] * MUL + 0.5;
		ikern_div += ikernel[i];
	}

	GP_ASSERT(ikern_div != 0);

	/* Create temporary buffers */
	gp_temp_alloc_create(temp, {{ len(pt.chanslist) }} * size * sizeof(int));

@         for c in pt.chanslist:
	int *{{ c.name }} = gp_temp_alloc_get(temp, size * sizeof(int));
@         end

	/* Do horizontal linear convolution */
	for (y = 0; y < (gp_coord)h_src; y++) {
		int yi = GP_MIN(y_src + y, (int)src->h - 1);

		/* Fetch the whole row */
		gp_pixel pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(src, 0, yi);

		int xi = x_src - kw/2;
		i = 0;

		/* Copy border pixel until the source image starts */
		while (xi <= 0 && i < size) {
@         for c in pt.chanslist:
			{{ c.name }}[i] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(pix, {{ c.name }}_gamma_lin);
@         end
			i++;
			xi++;
		}

		/* Use as much source image pixels as possible */
		while (xi < (int)src->w && i < size) {
			pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(src, xi, yi);

@         for c in pt.chanslist:
			{{ c.name }}[i] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(pix, {{ c.name }}_gamma_lin);
@         end

			i++;
			xi++;
		}

		/* Copy the rest the border pixel when we are out again */
		while (i < size) {
@         for c in pt.chanslist:
			{{ c.name }}[i] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(pix, {{ c.name }}_gamma_lin);
@         end

			i++;
		}

		for (x = 0; x < (gp_coord)w_src; x++) {
@         for c in pt.chanslist:
			int32_t {{ c.name }}_sum = MUL/2;
			int *p{{ c.name }} = {{ c.name }} + x;
@         end


			/* count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kw; i++) {
@         for c in pt.chanslist:
				{{ c.name }}_sum += (*p{{ c.name }}++) * ikernel[i];
@         end
			}

			/* divide the result */
@         for c in pt.chanslist:
			{{ c.name }}_sum /= ikern_div;
@         end

			/* and clamp just to be extra sure */
@         for c in pt.chanslist:
			gp_pixel {{ c.name }}_res = GP_CLAMP({{ c.name }}_sum, 0, (int){@ chan_lin_max(c) @});
@         end

			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x_dst + x, y_dst + y,
				GP_PIXEL_CREATE_{{ pt.name }}_ENC(
					{{ arr_to_params(pt.chan_names, "", "_res") }},
					{{ arr_to_params(pt.chan_names, "", "_gamma_enc") }}
				));
		}

		if (gp_progress_cb_report(callback, y, h_src, w_src)) {
			gp_temp_alloc_free(temp);
			return 1;
		}
	}

	gp_temp_alloc_free(temp);

	gp_progress_cb_done(callback);
	return 0;
}

@ end

int gp_filter_hlinear_convolution_raw(const gp_pixmap *src,
                                      gp_coord x_src, gp_coord y_src,
                                      gp_size w_src, gp_size h_src,
                                      gp_pixmap *dst,
                                      gp_coord x_dst, gp_coord y_dst,
                                      float kernel[], uint32_t kw, float kern_div,
				      gp_progress_cb *callback)
{
	GP_DEBUG(1, "Horizontal linear convolution kernel width %u "
	            "offset %ix%i rectangle %ux%u src->gamma %p dst->gamma %p",
		    kw, x_src, y_src, w_src, h_src, src->gamma, dst->gamma);

	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		return h_lin_conv_{{ pt.name }}(src, x_src, y_src, w_src, h_src,
                                                dst, x_dst, y_dst,
		                                kernel, kw, kern_div, callback);
	break;
@ end
	default:
		errno = EINVAL;
		return -1;
	}
}

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():

static int v_lin_conv_{{ pt.name }}(const gp_pixmap *src,
                                    gp_coord x_src, gp_coord y_src,
                                    gp_size w_src, gp_size h_src,
                                    gp_pixmap *dst,
                                    gp_coord x_dst, gp_coord y_dst,
                                    float kernel[], uint32_t kh, float kern_div,
                                    gp_progress_cb *callback)
{
	gp_coord x, y;
	uint32_t i;
	int ikernel[kh], ikern_div;
	uint32_t size = h_src + kh - 1;

	/* Fetch gamma tables */
	{@ fetch_gamma_lin(pt, 'src') @}
	{@ fetch_gamma_enc(pt, 'dst') @}

	/* Fetch maximal values for linearized channels */
	{@ fetch_chan_lin_max(pt, 'src') @}

	ikern_div = 0;
	(void) kern_div;

	for (i = 0; i < kh; i++) {
		ikernel[i] = kernel[i] * MUL + 0.5;
		ikern_div += ikernel[i];
	}

	GP_ASSERT(ikern_div != 0);

	/* Create temporary buffers */
	gp_temp_alloc_create(temp, {{ len(pt.chanslist) }} * size * sizeof(int));

@         for c in pt.chanslist:
	int *{{ c.name }} = gp_temp_alloc_get(temp, size * sizeof(int));
@         end

	/* Do vertical linear convolution */
	for (x = 0; x < (gp_coord)w_src; x++) {
		int xi = GP_MIN(x_src + x, (int)src->w - 1);

		/* Fetch the whole row */
		gp_pixel pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(src, xi, 0);

		int yi = y_src - kh/2;
		i = 0;

		/* Copy border pixel until the source image starts */
		while (yi <= 0 && i < size) {
@         for c in pt.chanslist:
			{{ c.name }}[i] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(pix, {{ c.name }}_gamma_lin);
@         end

			i++;
			yi++;
		}

		/* Use as much source image pixels as possible */
		while (yi < (int)src->h && i < size) {
			pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(src, xi, yi);

@         for c in pt.chanslist:
			{{ c.name }}[i] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(pix, {{ c.name }}_gamma_lin);
@         end

			i++;
			yi++;
		}

		/* Copy the rest the border pixel when we are out again */
		while (i < size) {
@         for c in pt.chanslist:
			{{ c.name }}[i] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(pix, {{ c.name }}_gamma_lin);
@         end

			i++;
		}

		for (y = 0; y < (gp_coord)h_src; y++) {
@         for c in pt.chanslist:
			int64_t {{ c.name }}_sum = MUL/2;
			int *p{{ c.name }} = {{ c.name }} + y;
@         end

			/* count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kh; i++) {
@         for c in pt.chanslist:
				{{ c.name }}_sum += (*p{{ c.name }}++) * ikernel[i];
@         end
			}

			/* divide the result */
@         for c in pt.chanslist:
			{{ c.name }}_sum /= ikern_div;
@         end

			/* and clamp just to be extra sure */
@         for c in pt.chanslist:
			gp_pixel {{ c.name }}_res = GP_CLAMP({{ c.name }}_sum, 0, (int){@ chan_lin_max(c) @});
@         end

			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x_dst + x, y_dst + y,
			                      GP_PIXEL_CREATE_{{ pt.name }}_ENC(
					      {{ arr_to_params(pt.chan_names, "", "_res") }},
					      {{ arr_to_params(pt.chan_names, "", "_gamma_enc") }}
					      ));
		}

		if (gp_progress_cb_report(callback, x, w_src, h_src)) {
			gp_temp_alloc_free(temp);
			return 1;
		}
	}

	gp_temp_alloc_free(temp);

	gp_progress_cb_done(callback);
	return 0;
}

@ end

int gp_filter_vlinear_convolution_raw(const gp_pixmap *src,
                                      gp_coord x_src, gp_coord y_src,
                                      gp_size w_src, gp_size h_src,
                                      gp_pixmap *dst,
                                      gp_coord x_dst, gp_coord y_dst,
                                      float kernel[], uint32_t kh, float kern_div,
                                      gp_progress_cb *callback)
{
	GP_DEBUG(1, "Vertical linear convolution kernel width %u "
	            "offset %ix%i rectangle %ux%u",
		    kh, x_src, y_src, w_src, h_src);

	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		return v_lin_conv_{{ pt.name }}(src, x_src, y_src, w_src, h_src,
                                                dst, x_dst, y_dst,
		                                kernel, kh, kern_div, callback);
	break;
@ end
	default:
		errno = EINVAL;
		return -1;
	}
}

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():

static int lin_conv_{{ pt.name }}(const gp_pixmap *src,
                                  gp_coord x_src, gp_coord y_src,
                                  gp_size w_src, gp_size h_src,
                                  gp_pixmap *dst,
                                  gp_coord x_dst, gp_coord y_dst,
                                  float kernel[], uint32_t kw, uint32_t kh,
                                  float kern_div, gp_progress_cb *callback)
{
	gp_coord x, y;
	unsigned int i, j;

	{@ fetch_gamma_lin(pt, "src") @}
	{@ fetch_gamma_enc(pt, "dst") @}
	{@ fetch_chan_lin_max(pt, 'src') @}

	/* Do linear convolution */
	for (y = 0; y < (gp_coord)h_src; y++) {
@         for c in pt.chanslist:
		uint32_t {{ c.name }}[kw][kh];
@         end
		gp_pixel pix;

		/* Prefill the buffer on the start */
		for (j = 0; j < kh; j++) {
			for (i = 0; i < kw - 1; i++) {
				int xi = x_src + i - kw/2;
				int yi = y_src + y + j - kh/2;

				xi = GP_CLAMP(xi, 0, (int)src->w - 1);
				yi = GP_CLAMP(yi, 0, (int)src->h - 1);

				pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(src, xi, yi);

@         for c in pt.chanslist:
				{{ c.name }}[i][j] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(pix, {{ c.name }}_gamma_lin);
@         end
			}
		}

		int idx = kw - 1;

		for (x = 0; x < (gp_coord)w_src; x++) {
@         for c in pt.chanslist:
			float {{ c.name }}_sum = 0;
@         end

			for (j = 0; j < kh; j++) {
				int xi = x_src + x + kw/2;
				int yi = y_src + y + j - kh/2;

				xi = GP_CLAMP(xi, 0, (int)src->w - 1);
				yi = GP_CLAMP(yi, 0, (int)src->h - 1);

				pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(src, xi, yi);

@         for c in pt.chanslist:
				{{ c.name }}[idx][j] = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}_LIN(pix, {{ c.name }}_gamma_lin);
@         end
			}

			/* Count the pixel value from neighbours weighted by kernel */
			for (i = 0; i < kw; i++) {
				int k;

				if ((int)i < idx + 1)
					k = kw - idx - 1 + i;
				else
					k = i - idx - 1;

				for (j = 0; j < kh; j++) {
@         for c in pt.chanslist:
					{{ c.name }}_sum += {{ c.name }}[i][j] * kernel[k + j * kw];
@         end
				}
			}

			/* divide the result */
@         for c in pt.chanslist:
			{{ c.name }}_sum /= kern_div;
@         end

			/* and clamp just to be extra sure */
@         for c in pt.chanslist:
			int {{ c.name }}_res = GP_CLAMP((int){{ c.name }}_sum, 0, (int){@ chan_lin_max(c) @});
@         end

			pix = GP_PIXEL_CREATE_{{ pt.name }}_ENC({{ arr_to_params(pt.chan_names, "", "_res") }}, {{ arr_to_params(pt.chan_names, '', '_gamma_enc') }});

			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x_dst + x, y_dst + y, pix);

			idx++;

			if (idx >= (int)kw)
				idx = 0;
		}

		if (gp_progress_cb_report(callback, y, h_src, w_src))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end

int gp_filter_linear_convolution_raw(const gp_pixmap *src,
                                     gp_coord x_src, gp_coord y_src,
                                     gp_size w_src, gp_size h_src,
                                     gp_pixmap *dst,
                                     gp_coord x_dst, gp_coord y_dst,
                                     float kernel[], uint32_t kw, uint32_t kh,
                                     float kern_div, gp_progress_cb *callback)
{
	GP_DEBUG(1, "Linear convolution kernel %ix%i rectangle %ux%u",
	            kw, kh, w_src, h_src);

	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		return lin_conv_{{ pt.name }}(src, x_src, y_src, w_src, h_src,
                                              dst, x_dst, y_dst,
		                              kernel, kw, kh, kern_div, callback);
	break;
@ end
	default:
		errno = EINVAL;
		return -1;
	}
}
