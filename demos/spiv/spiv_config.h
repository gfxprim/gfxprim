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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef __SPIV_CONFIG_H__
#define __SPIV_CONFIG_H__

#include <GP.h>

enum orientation {
	ROTATE_0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270,
};

enum zoom_strategy {
	/* Resize window to content size */
	ZOOM_WIN_RESIZABLE = 0x00,
	/* Do not change window size */
	ZOOM_WIN_FIXED = 0x01,

	/* Upscale image if window is bigger */
	ZOOM_IMAGE_UPSCALE = 0x01,
	/* Downscale image if window is smaller */
	ZOOM_IMAGE_DOWNSCALE = 0x02,
};

struct spiv_config {
	float slideshow_delay;
	enum orientation orientation;
	int win_strategy:2;
	int zoom_strategy:2;
	/* Maximal window size */
	unsigned int max_win_w;
	unsigned int max_win_h;

	int show_progress:1;
	int show_info:1;
	int floyd_steinberg:1;
	int timers:1;
	int full_screen:1;
	char backend_init[128];
	GP_PixelType emul_type;
};

extern struct spiv_config config;

static inline void config_win_toggle(void)
{
	config.win_strategy = !config.win_strategy;
}

static inline void config_upscale_toggle(void)
{
	config.zoom_strategy ^= ZOOM_IMAGE_UPSCALE;
}

static inline void config_downscale_toggle(void)
{
	config.zoom_strategy ^= ZOOM_IMAGE_DOWNSCALE;
}

int spiv_config_load(const char *path);

int spiv_config_parse_args(int argc, char *argv[]);

void spiv_config_print_help(void);

void spiv_config_print_man(void);

#endif /* __SPIV_CONFIG_H__ */
