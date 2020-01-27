@ include source.t
/*
 * Vertical Mirror alogorithm
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_debug.h>
#include <core/gp_get_put_pixel.h>
#include <filters/GP_Rotate.h>

@ for ps in pixelsizes:
static int rotate_90_{{ ps.suffix }}(const gp_pixmap *src, gp_pixmap *dst,
                                     gp_progress_cb *callback)
{
	uint32_t x, y;

	GP_DEBUG(1, "Rotating image by 90 %ux%u", src->w, src->h);

	for (x = 0; x < src->w; x++) {
		for (y = 0; y < src->h; y++) {
			uint32_t yr = src->h - y - 1;
			gp_putpixel_raw_{{ ps.suffix }}(dst, yr, x, gp_getpixel_raw_{{ ps.suffix }}(src, x, y));
		}

		if (gp_progress_cb_report(callback, x, src->w, src->h))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
static int rotate_90(const gp_pixmap *src, gp_pixmap *dst,
                     gp_progress_cb *callback)
{
	GP_FN_RET_PER_BPP_PIXMAP(rotate_90, src, src, dst, callback);
	return 1;
}

int gp_filter_rotate_90(const gp_pixmap *src, gp_pixmap *dst,
                        gp_progress_cb *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(src->w <= dst->h && src->h <= dst->w,
	          "Destination is not large enough");

	if (rotate_90(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

gp_pixmap *gp_filter_rotate_90_alloc(const gp_pixmap *src,
                                     gp_progress_cb *callback)
{
	gp_pixmap *res;

	res = gp_pixmap_alloc(src->h, src->w, src->pixel_type);

	if (res == NULL)
		return NULL;

	if (rotate_90(src, res, callback)) {
		GP_DEBUG(1, "Operation aborted");
		gp_pixmap_free(res);
		return NULL;
	}

	return res;
}

@ def swap_pixels(ps, src, dst, x0, y0, x1, y1):
gp_pixel pix0 = gp_getpixel_raw_{{ ps.suffix }}({{ src }}, {{ x0 }}, {{ y0 }});
gp_pixel pix1 = gp_getpixel_raw_{{ ps.suffix }}({{ src }}, {{ x1 }}, {{ y1 }});
gp_putpixel_raw_{{ ps.suffix }}({{ dst }}, {{ x0 }}, {{ y0 }}, pix1);
gp_putpixel_raw_{{ ps.suffix }}({{ dst }}, {{ x1 }}, {{ y1 }}, pix0);
@ end
@
@ for ps in pixelsizes:
static int rotate_180_{{ ps.suffix }}(const gp_pixmap *src, gp_pixmap *dst,
                                      gp_progress_cb *callback)
{
	uint32_t x, y;

	GP_DEBUG(1, "Rotating image by 180 %ux%u", src->w, src->h);

	for (x = 0; x < src->w; x++) {
		for (y = 0; y < src->h; y++) {
			uint32_t xr = src->w - x - 1;
			uint32_t yr = src->h - y - 1;

			{@ swap_pixels(ps, 'src', 'dst', 'x', 'y', 'xr', 'yr') @}
		}

		if (gp_progress_cb_report(callback, x, src->w, src->h))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
static int rotate_180(const gp_pixmap *src, gp_pixmap *dst,
                      gp_progress_cb *callback)
{
	GP_FN_RET_PER_BPP_PIXMAP(rotate_180, src, src, dst, callback);
	return 1;
}

int gp_filter_rotate_180(const gp_pixmap *src, gp_pixmap *dst,
                         gp_progress_cb *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(src->w <= dst->w && src->h <= dst->h,
	          "Destination is not large enough");

	if (rotate_180(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

gp_pixmap *gp_filter_rotate_180_alloc(const gp_pixmap *src,
                                      gp_progress_cb *callback)
{
	gp_pixmap *res;

	res = gp_pixmap_copy(src, 0);

	if (res == NULL)
		return NULL;

	if (rotate_180(src, res, callback)) {
		GP_DEBUG(1, "Operation aborted");
		gp_pixmap_free(res);
		return NULL;
	}

	return res;
}

@ for ps in pixelsizes:
static int rotate_270_{{ ps.suffix }}(const gp_pixmap *src, gp_pixmap *dst,
                                      gp_progress_cb *callback)
{
	uint32_t x, y;

	GP_DEBUG(1, "Rotating image by 270 %ux%u", src->w, src->h);

	for (x = 0; x < src->w; x++) {
		for (y = 0; y < src->h; y++) {
				uint32_t xr = src->w - x - 1;
				gp_putpixel_raw_{{ ps.suffix }}(dst, y, xr, gp_getpixel_raw_{{ ps.suffix }}(src, x, y));
		}

		if (gp_progress_cb_report(callback, x, src->w, src->h))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
static int rotate_270(const gp_pixmap *src, gp_pixmap *dst,
                      gp_progress_cb *callback)
{
	GP_FN_RET_PER_BPP_PIXMAP(rotate_270, src, src, dst, callback);
	return 1;
}

int gp_filter_rotate_270(const gp_pixmap *src, gp_pixmap *dst,
                         gp_progress_cb *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(src->w <= dst->h && src->h <= dst->w,
	          "Destination is not large enough");

	if (rotate_270(src, dst, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}

gp_pixmap *gp_filter_rotate_270_alloc(const gp_pixmap *src,
                                      gp_progress_cb *callback)
{
	gp_pixmap *res;

	res = gp_pixmap_alloc(src->h, src->w, src->pixel_type);

	if (res == NULL)
		return NULL;

	if (rotate_270(src, res, callback)) {
		GP_DEBUG(1, "Operation aborted");
		gp_pixmap_free(res);
		return NULL;
	}

	return res;
}
