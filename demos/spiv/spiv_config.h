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

struct spiv_config {
	float slideshow_delay;
	enum orientation orientation;
	int show_progress:1;
	int show_info:1;
	int floyd_steinberg:1;
	int timers:1;
	char backend_init[128];
	GP_PixelType emul_type;
};

extern struct spiv_config config;

int spiv_config_load(const char *path);

int spiv_config_parse_args(int argc, char *argv[]);

void spiv_config_print_help(void);

void spiv_config_print_man(void);

#endif /* __SPIV_CONFIG_H__ */
