@ include header.t
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_blend_pixel.gen.h
 * @brief Composites one pixel over another.
 *
 * Blends source over destination. The source covers the destination in
 * proportion to its own alpha and what is left of the destination shows
 * through the rest.
 *
 * Both pixels are of the same type, since a shape is painted into the pixmap
 * it is rendered into, and the arithmetics is done in the native channel
 * depths of the pixel type.
 */

#include <core/gp_pixel.h>

@ for pt in pixeltypes:
@     if pt.is_alpha():
/**
 * @brief Composites a {{ pt.name }} source pixel over a {{ pt.name }} destination pixel.
 *
 * @param src A source pixel.
 * @param dst A destination pixel.
 * @return The composited pixel.
 */
static inline gp_pixel gp_blend_pixel_{{ pt.name }}(gp_pixel src, gp_pixel dst)
{
@         A = pt.chans['A']
@         chan_bits = max(c.size for c in pt.chanslist if not c.is_alpha)
@         wtype = 'uint32_t' if chan_bits + 2 * A.size + 1 <= 32 else 'uint64_t'
	{{ wtype }} sa = GP_PIXEL_GET_A_{{ pt.name }}(src);
	{{ wtype }} da = GP_PIXEL_GET_A_{{ pt.name }}(dst);
	{{ wtype }} sw, dw, ow, oa, {{ ', '.join(c.name for c in pt.chanslist if not c.is_alpha) }};

	if (!sa)
		return dst;

	if (sa == {{ A.C_max }})
		return src;

	/*
	 * The weights of the source and of what is left of the destination once
	 * the source has covered it, both scaled by the alpha maximum so that
	 * nothing is rounded until the very end.
	 */
	sw = sa * {{ A.C_max }};
	dw = da * ({{ A.C_max }} - sa);
	ow = sw + dw;

	oa = (ow + {{ hex(A.max // 2) }}) / {{ A.C_max }};

@         for c in pt.chanslist:
@             if not c.is_alpha:
	{{ c.name }} = (GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(src) * sw +
	     GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(dst) * dw + ow/2) / ow;
@         end

	return GP_PIXEL_CREATE_{{ pt.name }}({{ ', '.join('oa' if c == 'A' else c for c in pt.chan_names) }});
}

@ end
/**
 * @brief Composites a source pixel over a destination pixel.
 *
 * A pixel type without an alpha channel has an opaque source, which is
 * returned as it is.
 *
 * @param src A source pixel.
 * @param dst A destination pixel.
 * @param type The pixel type of both.
 * @return The composited pixel.
 */
static inline gp_pixel gp_blend_pixel(gp_pixel src, gp_pixel dst,
                                      gp_pixel_type type)
{
	switch (type) {
@ for pt in pixeltypes:
@     if pt.is_alpha():
	case GP_PIXEL_{{ pt.name }}:
		return gp_blend_pixel_{{ pt.name }}(src, dst);
@ end
	default:
		return src;
	}
}
