//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>

*/

#include <gfxprim.h>

int main(void)
{
	gp_backend *backend;

	backend = gp_linux_drm_init("/dev/dri/card1", 0);

	gp_pixel red = gp_rgb_to_pixmap_pixel(0xff, 0x00, 0x00, backend->pixmap);

	gp_fill(backend->pixmap, red);
	gp_backend_flip(backend);

	sleep(1);

	gp_backend_exit(backend);

	return 0;
}
