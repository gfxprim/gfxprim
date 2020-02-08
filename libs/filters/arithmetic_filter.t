@ def filter_arithmetic(name, filter_op, opts='', params=''):
#include <core/gp_pixmap.h>
#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_debug.h>
#include <filters/gp_filter.h>
#include <filters/gp_arithmetic.h>

@     for pt in pixeltypes:
@         if not pt.is_unknown():
static int filter_{{ name }}_{{ pt.name }}(const gp_pixmap *src_a, const gp_pixmap *src_b,
	gp_pixmap *dst, {{ maybe_opts_r(opts) }}gp_progress_cb *callback)
{
	uint32_t x, y, w, h;

	w = GP_MIN(src_a->w, src_b->w);
	h = GP_MIN(src_a->h, src_b->h);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			gp_pixel pix_a = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src_a, x, y);
			gp_pixel pix_b = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src_b, x, y);

@             for c in pt.chanslist:
			int32_t {{ c.name }}_A = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(pix_a);
			int32_t {{ c.name }}_B = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(pix_b);
@             end

@             for c in pt.chanslist:
			int32_t {{ c.name }};
			{@ filter_op(c.name, c.size) @}
@             end

			gp_pixel pix;
			pix = GP_PIXEL_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names) }});

			gp_putpixel_raw_{{ pt.pixelsize.suffix }}(dst, x, y, pix);
		}

		if (gp_progress_cb_report(callback, y, h, w))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

@     end
@
int gp_filter_{{ name }}_raw(const gp_pixmap *src_a, const gp_pixmap *src_b,
	gp_pixmap *dst{{ maybe_opts_l(opts) }}, gp_progress_cb *callback)
{
	GP_DEBUG(1, "Running filter {{ name }}");

	switch (src_a->pixel_type) {
@     for pt in pixeltypes:
@         if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		return filter_{{ name }}_{{ pt.name }}(src_a, src_b, dst{{ maybe_opts_l(params) }}, callback);
@     end
	default:
	break;
	}

	return 1;
}

int gp_filter_{{ name }}(const gp_pixmap *src_a, const gp_pixmap *src_b,
                         gp_pixmap *dst{{ maybe_opts_l(opts) }},
                         gp_progress_cb *callback)
{
	gp_size w = GP_MIN(src_a->w, src_b->w);
	gp_size h = GP_MIN(src_a->h, src_b->h);

	GP_ASSERT(src_a->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(w <= dst->w && h <= dst->h,
	          "Destination is not big enough");

	if (gp_filter_{{ name }}_raw(src_a, src_b, dst{{ maybe_opts_l(params) }}, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}


gp_pixmap *gp_filter_{{ name }}_alloc(const gp_pixmap *src_a, const gp_pixmap *src_b,
	                              {{ maybe_opts_r(opts) }}gp_progress_cb *callback)
{
	gp_pixmap *res;

	GP_ASSERT(src_a->pixel_type == src_b->pixel_type,
	          "Pixel types for sources must match.");

	gp_size w = GP_MIN(src_a->w, src_b->w);
	gp_size h = GP_MIN(src_a->h, src_b->h);

	res = gp_pixmap_alloc(w, h, src_a->pixel_type);

	if (res == NULL)
		return NULL;

	if (gp_filter_{{ name }}_raw(src_a, src_b, res{{ maybe_opts_l(params) }}, callback)) {
		GP_DEBUG(1, "Operation aborted");

		gp_pixmap_free(res);

		return NULL;
	}

	return res;
}
@ end
