@ include source.t
@ include WritePixels.t
/*
 * Optimized fill functions.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_Pixmap.h"
#include "core/GP_WritePixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_FnPerBpp.h"
#include "core/GP_Fill.h"

@ for ps in pixelsizes:
static void fill_{{ ps.suffix }}(gp_pixmap *ctx, gp_pixel val)
{
	unsigned int y;

	for (y = 0; y < ctx->h; y++) {
@     if ps.suffix in optimized_writepixels:
		void *start = GP_PIXEL_ADDR(ctx, 0, y);
@         if ps.needs_bit_endian():
		gp_write_pixels_{{ ps.suffix }}(start, 0, ctx->w, val);
@         else:
		gp_write_pixels_{{ ps.suffix }}(start, ctx->w, val);
@     else:
		unsigned int x;

		for (x = 0; x < ctx->w; x++)
			gp_putpixel_raw_{{ ps.suffix }}(ctx, x, y, val);
@     end
	}
}

@ end

void gp_fill(gp_pixmap *ctx, gp_pixel val)
{
	GP_FN_PER_BPP_PIXMAP(fill, ctx, ctx, val);
}
