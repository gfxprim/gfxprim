// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   Simple GFX Example.

  */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <gfxprim.h>

/*
 * Generate color depending on distance from center
 *
 * We could do this only and only because the pixmap
 * pixel type is fixed to GP_PIXEL_RGB888.
 */
static gp_pixel do_color(int xc, int yc, float x, float y)
{
	float dx = GP_ABS(1.00 * xc - x)/(2*xc);
	float dy = GP_ABS(1.00 * yc - y)/(2*yc);

	float dc = sqrt(dx*dx + dy*dy);

	int rmask = 0;
	int gmask = 0xff * sqrt(dc);
	int bmask = 0xff * (1 - dc);

	if (dc < 0.1) {
		rmask = 0xff * (1 - dc);
		bmask = 0x00;
	}

	if (dc > 0.55) {
		rmask = 0xff * dc;
		gmask /= 6;
		bmask = 0x00;
	}

	return bmask | (gmask<<8) | (rmask << 16);
}

static void draw(gp_pixmap *img, int level, float x0, float y0, float x1, float y1)
{
	if (level == 0) {
		gp_pixel pixel;

		pixel = do_color(img->w/2, img->h/2, 1.00 * (x0+x1)/2, 1.00 * (y0 + y1)/2);

		gp_line(img, x0, y0, x1, y1, pixel);

		return;
	}

	/* Compute varation of Koch curve */
	float x34 = (x0 + 3 * x1) / 4;
	float y34 = (y0 + 3 * y1) / 4;

	float x14 = (3 * x0 + x1) / 4;
	float y14 = (3 * y0 + y1) / 4;

	float x12 = (x0 + x1)/2;
	float y12 = (y0 + y1)/2;

	float dx = (x1 - x0)/4;
	float dy = (y1 - y0)/4;

	draw(img, level - 1, x0, y0, x14, y14);
	draw(img, level - 1, x14, y14, x14 - dy, y14 + dx);
	draw(img, level - 1, x14 - dy, y14 + dx, x12 - dy, y12 + dx);
	draw(img, level - 1, x12 - dy, y12 + dx, x12, y12);
	draw(img, level - 1, x12, y12, x12 + dy, y12 - dx);
	draw(img, level - 1, x12 + dy, y12 - dx, x34 + dy, y34 - dx);
	draw(img, level - 1, x34 + dy, y34 - dx, x34, y34);
	draw(img, level - 1, x1, y1, x34, y34);
}

int main(void)
{
	gp_pixmap *img;

	/* Create RGB 24 bit image */
	img = gp_pixmap_alloc(600, 600, GP_PIXEL_RGB888);

	if (img == NULL) {
		fprintf(stderr, "Failed to allocate pixmap");
		return 1;
	}

	/* Clean up the bitmap */
	gp_fill(img, 0);

	/* Draw a fractal */
	draw(img, 4, 0, 0, img->w - 1, img->h - 1);
	draw(img, 4, 0, img->h - 1, img->w - 1, 0);

	if (gp_save_png(img, "out.png", NULL)) {
		fprintf(stderr, "Failed to save image %s", strerror(errno));
		return 1;
	}

	/* Cleanup */
	gp_pixmap_free(img);

	return 0;
}
