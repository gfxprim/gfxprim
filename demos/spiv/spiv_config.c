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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "image_actions.h"
#include "spiv_config.h"

/*
 * These are default config values, you can hardcompile yours here.
 */
struct spiv_config config = {
	.slideshow_delay = 0,
	.show_info = 0,
	.backend_init = "X11",
	.emul_type = GP_PIXEL_UNKNOWN,
};

static int set_zoom_strategy(struct cfg_opt *self, unsigned int lineno)
{
	(void) lineno;
	//TODO!!!
	printf("ZoomStrategy = %s\n", self->val);

	return 0;
}

static int set_action(struct cfg_opt *self, unsigned int lineno)
{
	(void) lineno;
	image_action_set(atoi(self->key), self->val);
	return 0;
}

static int set_opt(struct cfg_opt *self, unsigned int lineno)
{
	(void) self;
	(void) lineno;

	switch (self->opt) {
	case 'f':
		config.floyd_steinberg = 1;
	break;
	case 'i':
		config.show_info = 1;
	break;
	case 'p':
		config.show_progress = 1;
	break;
	case 't':
		config.timers = 1;
	break;
	}

	return 0;
}

static int set_orientation(struct cfg_opt *self, unsigned int lineno)
{
	if (!strcmp("0", self->val)) {
		config.orientation = ROTATE_0;
		return 0;
	}

	if (!strcmp("90", self->val)) {
		config.orientation = ROTATE_90;
		return 0;
	}

	if (!strcmp("180", self->val)) {
		config.orientation = ROTATE_180;
		return 0;
	}

	if (!strcmp("270", self->val)) {
		config.orientation = ROTATE_270;
		return 0;
	}

	fprintf(stderr, "ERROR: %u: Invalid orientation '%s'\n",
	        lineno, self->val);
	return 1;
}

static int set_backend_init(struct cfg_opt *self, unsigned int lineno)
{
	if (strlen(self->val) + 1 >= sizeof(config.backend_init)) {
		fprintf(stderr, "ERROR: %u: Backend init string too long\n",
		        lineno);
		return 1;
	}

	strcpy(config.backend_init, self->val);

	return 0;
}

static int set_slideshow(struct cfg_opt *self, unsigned int lineno)
{
	config.slideshow_delay = atof(self->val);

	if (config.slideshow_delay == 0) {
		fprintf(stderr, "ERROR: %u: Invalid slideshow delay '%s'\n",
		        lineno, self->val);
		return 1;
	}

	return 0;
}

static int set_emulation(struct cfg_opt *self, unsigned int lineno)
{
	config.emul_type = GP_PixelTypeByName(optarg);

	if (config.emul_type == GP_PIXEL_UNKNOWN) {
		fprintf(stderr, "ERROR: %u: Invalid pixel type '%s'\n",
		        lineno, self->val);
		return 1;
	}

	return 0;
}

static int help(struct cfg_opt *self, unsigned int lineno)
{
	(void) self;
	(void) lineno;

	print_help();
	exit(0);
}

static int man(struct cfg_opt *self, unsigned int lineno)
{
	(void) self;
	(void) lineno;

	print_man();
	exit(0);
}

struct cfg_opt spiv_opts[] = {
	{.name_space = NULL,
	 .key = NULL,
	 .opt = 'h',
	 .opt_long = "help",
	 .opt_has_value = 0,
	 .set = help,
	 .help = "Shows this help",
	},

	{.name_space = "Gui",
	 .key = "ShowInfo",
	 .opt = 'i',
	 .opt_long = "show-info",
	 .opt_has_value = 0,
	 .set = set_opt,
	 .help = "Show image info such as filename, size, etc...",
	},
	{.name_space = "Gui",
	 .key = "ShowProgress",
	 .opt = 'p',
	 .opt_long = "show-progress",
	 .set = set_opt,
	 .help = "Show progress bar when loading/resampling/... images",
	},
	{.name_space = "Gui",
	 .key = "SlideshowDelay",
	 .opt = 's',
	 .opt_long = "slideshow-delay",
	 .opt_has_value = 1,
	 .set = set_slideshow,
	 .help = "Delay between images in seconds (float) for slideshow",
	},
	{.name_space = "Gui",
	 .key = "UseFloydSteinberg",
	 .opt = 'f',
	 .opt_long = "floyd-steinberg",
	 .opt_has_value = 0,
	 .set = set_opt,
	 .help = "Turn on Floyd-Steinberg dithering",
	},
	{.name_space = "Gui",
	 .key = "Orientation",
	 .opt = 'o',
	 .opt_long = "orientation",
	 .opt_has_value = 1,
	 .set = set_orientation,
	 .help = "Orientation, one of 0, 90, 180, 270",
	},
	{.name_space = "Gui",
	 .key = "BackendInit",
	 .opt = 'b',
	 .opt_long = "backend-init",
	 .opt_has_value = 1,
	 .set = set_backend_init,
	 .help = "Backend init string, set it to 'help' for more info",
	},

	{.name_space = "Zoom",
	 .key = "ZoomStrategy",
	 .opt = 'z',
	 .opt_long = "zoom-strategy",
	 .opt_has_value = 1,
	 .set = set_zoom_strategy,
	 .help = "Zoom strategy",
	},


	{.name_space = "Actions",
	 .key = "1",
	 .opt = '1',
	 .opt_long = "action-1",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "2",
	 .opt = '2',
	 .opt_long = "action-2",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "3",
	 .opt = '3',
	 .opt_long = "action-3",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "4",
	 .opt = '4',
	 .opt_long = "action-4",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "5",
	 .opt = '5',
	 .opt_long = "action-5",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "6",
	 .opt = '6',
	 .opt_long = "action-6",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "7",
	 .opt = '7',
	 .opt_long = "action-7",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "8",
	 .opt = '8',
	 .opt_long = "action-8",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "9",
	 .opt = '9',
	 .opt_long = "action-9",
	 .opt_has_value = 1,
	 .set = set_action,
	},
	{.name_space = "Actions",
	 .key = "10",
	 .opt = '0',
	 .opt_long = "action-10",
	 .opt_has_value = 1,
	 .set = set_action,
	 .help = "Sets command line for action 1-10",
	},

	{.name_space = "Devel",
	 .key = "Timers",
	 .opt = 't',
	 .opt_long = "timers",
	 .opt_has_value = 0,
	 .set = set_opt,
	 .help = "Turns on cpu and wall clock measurement (printed to stdout)",
	},
	{.name_space = "Devel",
	 .key = "BackendEmulation",
	 .opt = 'e',
	 .opt_long = "backend-emulation",
	 .opt_has_value = 1,
	 .set = set_emulation,
	 .help = "Emulate different backend pixel type (G1, G2, RGB555, ...)",
	},
	{.name_space = "Devel",
	 .key = NULL,
	 .opt_long = "print-man",
	 .opt_has_value = 0,
	 .set = man,
	 .help = "Prints spiv man page to stdout",
	},

	{NULL}
};

int spiv_config_load(const char *path)
{
	return cfg_load(spiv_opts, path);
}

int spiv_config_parse_args(int argc, char *argv[])
{
	return cfg_getopt(spiv_opts, argc, argv);
}

void spiv_config_print_help(void)
{
	cfg_print_help(spiv_opts);
}

void spiv_config_print_man(void)
{
	cfg_print_man(spiv_opts);
}
