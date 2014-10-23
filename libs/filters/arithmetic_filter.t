@ def filter_arithmetic(name, filter_op, opts='', params=''):
#include "core/GP_Context.h"
#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Debug.h"
#include "filters/GP_Filter.h"
#include "filters/GP_Arithmetic.h"

@     for pt in pixeltypes:
@         if not pt.is_unknown():
static int GP_Filter{{ name }}_{{ pt.name }}(const GP_Context *src_a, const GP_Context *src_b,
	GP_Context *dst, {{ maybe_opts_r(opts) }}GP_ProgressCallback *callback)
{
	uint32_t x, y, w, h;

	w = GP_MIN(src_a->w, src_b->w);
	h = GP_MIN(src_a->h, src_b->h);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			GP_Pixel pix_a = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src_a, x, y);
			GP_Pixel pix_b = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src_b, x, y);

@             for c in pt.chanslist:
			int32_t {{ c.name }}_A = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix_a);
			int32_t {{ c.name }}_B = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix_b);
@             end

@             for c in pt.chanslist:
			int32_t {{ c.name }};
			{@ filter_op(c.name, c.size) @}
@             end

			GP_Pixel pix;
			pix = GP_Pixel_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names) }});

			GP_PutPixel_Raw_{{ pt.pixelsize.suffix }}(dst, x, y, pix);
		}

		if (GP_ProgressCallbackReport(callback, y, h, w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@     end
@
int GP_Filter{{ name }}_Raw(const GP_Context *src_a, const GP_Context *src_b,
	GP_Context *dst{{ maybe_opts_l(opts) }}, GP_ProgressCallback *callback)
{
	GP_DEBUG(1, "Running filter {{ name }}");

	switch (src_a->pixel_type) {
@     for pt in pixeltypes:
@         if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		return GP_Filter{{ name }}_{{ pt.name }}(src_a, src_b, dst{{ maybe_opts_l(params) }}, callback);
@     end
	default:
	break;
	}

	return 1;
}

int GP_Filter{{ name }}(const GP_Context *src_a, const GP_Context *src_b,
                        GP_Context *dst{{ maybe_opts_l(opts) }},
                        GP_ProgressCallback *callback)
{
	GP_Size w = GP_MIN(src_a->w, src_b->w);
	GP_Size h = GP_MIN(src_a->h, src_b->h);

	GP_ASSERT(src_a->pixel_type == dst->pixel_type,
	          "The src and dst pixel types must match");
	GP_ASSERT(w <= dst->w && h <= dst->h,
	          "Destination is not big enough");

	if (GP_Filter{{ name }}_Raw(src_a, src_b, dst{{ maybe_opts_l(params) }}, callback)) {
		GP_DEBUG(1, "Operation aborted");
		return 1;
	}

	return 0;
}


GP_Context *GP_Filter{{ name }}Alloc(const GP_Context *src_a, const GP_Context *src_b,
	                            {{ maybe_opts_r(opts) }}GP_ProgressCallback *callback)
{
	GP_Context *res;

	GP_ASSERT(src_a->pixel_type == src_b->pixel_type,
	          "Pixel types for sources must match.");

	GP_Size w = GP_MIN(src_a->w, src_b->w);
	GP_Size h = GP_MIN(src_a->h, src_b->h);

	res = GP_ContextAlloc(w, h, src_a->pixel_type);

	if (res == NULL)
		return NULL;

	if (GP_Filter{{ name }}_Raw(src_a, src_b, res{{ maybe_opts_l(params) }}, callback)) {
		GP_DEBUG(1, "Operation aborted");

		GP_ContextFree(res);

		return NULL;
	}

	return res;
}
@ end
