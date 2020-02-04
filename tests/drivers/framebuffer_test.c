/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Backend test.

   1. Draw to the backend
   2. Sleep
   3. Exit

  */

#include <math.h>

#include <gfxprim.h>
#include <backends/gp_backends.h>

static void draw(gp_pixmap *pixmap, int x, int y, int l)
{
	int x2, y2, x3, y3;

	gp_pixel red, blue, green;

	red = gp_rgb_to_pixmap_pixel(255,   0,   0, pixmap);
	blue = gp_rgb_to_pixmap_pixel(0,   0, 255, pixmap);
	green = gp_rgb_to_pixmap_pixel(0, 255,   0, pixmap);

	x2 = x + l/2;
	y2 = y + sqrt(2)/2 * l;
	x3 = x - l/2;
	y3 = y2;

	gp_fill_triangle(pixmap,  x,  y, x2, y2, x + l, y, red);
	gp_fill_triangle(pixmap,  x,  y, x3, y3, x - l, y, green);
	gp_fill_triangle(pixmap, x2, y2, x3, y3, x, y + sqrt(2) * l, blue);
}

int main(void)
{
	gp_backend *backend;
	gp_pixmap *pixmap;

	gp_set_debug_level(10);

	backend = gp_linux_fb_init("/dev/fb0", 0);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize framebuffer\n");
		return 1;
	}

	pixmap = backend->pixmap;

	gp_pixel gray, black;

	gray = gp_rgb_to_pixmap_pixel(200, 200, 200, pixmap);
	black = gp_rgb_to_pixmap_pixel(0, 0, 0, pixmap);

	const char *text = "Framebuffer test";

	gp_fill(pixmap, gray);
	gp_line(pixmap, 0, 0, pixmap->w, pixmap->h, black);
	gp_line(pixmap, 0, pixmap->h, pixmap->w, 0, black);
	gp_text(pixmap, NULL,
	        (pixmap->w - gp_text_width(NULL, text))/2,
		16, GP_ALIGN_RIGHT|GP_VALIGN_BELOW, black, gray, text);


	draw(pixmap, pixmap->w / 2, 2.00 * pixmap->h / 3, 60);

	gp_backend_flip(backend);

	sleep(10);

	gp_backend_exit(backend);

	return 0;
}
