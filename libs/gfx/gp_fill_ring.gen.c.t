@ include source.t
/*
 * A filled circle drawing algorithm.
 *
 * Copyright (C) 2009-2012 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_transform.h>
#include <core/gp_fn_per_bpp.h>
#include <gfx/gp_hline.h>
#include <gfx/gp_circle.h>
#include <gfx/gp_circle_seg.h>

/*
 * A filled ring drawing algorithm.
 *
 * A filled ring is drawn in the same way as circle but we 'draw' two circles
 * at a time and fill the space between them.
 */
@ for ps in pixelsizes:

static void fill_ring_seg_{{ ps.suffix }}(gp_pixmap *pixmap,
	gp_coord xcenter, gp_coord ycenter, int inner_r, int outer_r,
	uint8_t seg_flag, gp_pixel pixel)
{
	int inner_x = 0;
	int outer_x = 0;
	int outer_err = -outer_r;
	int inner_err = -inner_r;
	int y;

	for (y = outer_r; y >= 0; y--) {

		while (outer_err < 0) {
			outer_err += 2*outer_x + 1;
			outer_x++;
		}
		outer_err += -2*y + 1;

		if (y < inner_r && y > -inner_r) {
			while (inner_err < 0) {
				inner_err += 2*inner_x + 1;
				inner_x++;
			}
			inner_err += -2*y + 1;

			if (seg_flag & GP_CIRCLE_SEG1)
				gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter + inner_x, xcenter+outer_x-1, ycenter-y, pixel);

			if (seg_flag & GP_CIRCLE_SEG2)
				gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-outer_x+1, xcenter-inner_x, ycenter-y, pixel);

			if (seg_flag & GP_CIRCLE_SEG3)
				gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-outer_x+1, xcenter-inner_x, ycenter+y, pixel);

			if (seg_flag & GP_CIRCLE_SEG4)
				gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter + inner_x, xcenter+outer_x-1, ycenter+y, pixel);
		} else {
			if (seg_flag & GP_CIRCLE_SEG1)
				gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter, xcenter+outer_x-1, ycenter-y, pixel);

			if (seg_flag & GP_CIRCLE_SEG2)
				gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-outer_x+1, xcenter, ycenter-y, pixel);

			if (seg_flag & GP_CIRCLE_SEG3)
				gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter-outer_x+1, xcenter, ycenter+y, pixel);

			if (seg_flag & GP_CIRCLE_SEG4)
				gp_hline_raw_{{ ps.suffix }}(pixmap, xcenter, xcenter+outer_x-1, ycenter+y, pixel);
		}
	}
}

@ end

void gp_fill_ring_seg_raw(gp_pixmap *pixmap, gp_coord xcenter, gp_coord ycenter,
                          gp_size r1, gp_size r2, uint8_t seg_flag, gp_pixel pixel)
{
	GP_CHECK_PIXMAP(pixmap);

	if (r1 > r2)
		GP_SWAP(r1, r2);

	GP_FN_PER_BPP_PIXMAP(fill_ring_seg, pixmap, pixmap,
	                     xcenter, ycenter, r1, r2, seg_flag, pixel);
}
