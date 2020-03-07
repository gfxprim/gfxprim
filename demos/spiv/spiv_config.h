// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef __SPIV_CONFIG_H__
#define __SPIV_CONFIG_H__

#include <gfxprim.h>

enum orientation {
	ROTATE_0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270,
	ROTATE_360,
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
	/* orientation set by user */
	enum orientation orientation;
	/* combined orientation (user orientation + EXIF rotation) */
	enum orientation combined_orientation;
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
	int exif_autorotate:1;
	char backend_init[128];
	gp_pixel_type emul_type;

	/* Font information */
	gp_text_style *style;
	char *font_path;
	unsigned int font_height;
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
