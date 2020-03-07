// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Pretty print function for pixel and pixmap.

 */

#include <gfxprim.h>

int main(void)
{
	gp_pixmap *pixmap = gp_pixmap_alloc(100, 100, GP_PIXEL_RGB888);
	gp_pixel pix = ~(gp_pixel)0;

	/* Pretty prints pixel values */
	gp_pixel_print(pix, GP_PIXEL_RGB888);

	/* Pretty prints pixmap info */
	gp_pixmap_print_info(pixmap);

	return 0;
}
