@ include source.t
/*
 * Generic Point filer
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_debug.h>

#include <filters/gp_apply_tables.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
static int apply_tables_{{ pt.name }}(const gp_pixmap *const src,
                                      gp_coord x_src, gp_coord y_src,
                                      gp_size w_src, gp_size h_src,
                                      gp_pixmap *dst,
                                      gp_coord x_dst, gp_coord y_dst,
                                      const gp_filter_tables *const tables,
                                      gp_progress_cb *callback)
{
	GP_DEBUG(1, "Point filter %ux%u", w_src, h_src);

	unsigned int x, y;

@         for c in pt.chanslist:
	gp_pixel {{ c.name }};
@         end

	for (y = 0; y < h_src; y++) {
		for (x = 0; x < w_src; x++) {
			unsigned int src_x = x_src + x;
			unsigned int src_y = y_src + y;
			unsigned int dst_x = x_dst + x;
			unsigned int dst_y = y_dst + y;

			gp_pixel pix = gp_getpixel_raw_{{ pt.pixelpack.suffix }}(src, src_x, src_y);

@         for c in pt.chanslist:
			{{ c.name }} = GP_PIXEL_GET_{{ c[0] }}_{{ pt.name }}(pix);
			{{ c.name }} = tables->table[{{ c.idx }}][{{ c.name }}];
@         end

			pix = GP_PIXEL_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names) }});
			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, dst_x, dst_y, pix);
		}

		if (gp_progress_cb_report(callback, y, h_src, w_src)) {
			errno = ECANCELED;
			return 1;
		}
	}

	gp_progress_cb_done(callback);

	return 0;
}

@ end
@
int gp_filter_tables_apply(const gp_pixmap *const src,
                           gp_coord x_src, gp_coord y_src,
                           gp_size w_src, gp_size h_src,
                           gp_pixmap *dst,
                           gp_coord x_dst, gp_coord y_dst,
                           const gp_filter_tables *const tables,
                           gp_progress_cb *callback)
{
	GP_ASSERT(src->pixel_type == dst->pixel_type);
	//TODO: Assert size

	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		return apply_tables_{{ pt.name }}(src, x_src, y_src,
		                                  w_src, h_src, dst,
		                                  x_dst, y_dst,
		                                  tables, callback);
	break;
@ end
	default:
		errno = EINVAL;
		return -1;
	}
}
