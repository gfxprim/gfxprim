// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "image_actions.h"
#include "spiv_config.h"
#include "spiv_help.h"

/*
 * These are default config values, you can hardcompile yours here.
 */
struct spiv_config config = {
	.slideshow_delay = 0,
	.show_info = 0,
	.backend_init = "X11",
	.emul_type = GP_PIXEL_UNKNOWN,
	.zoom_strategy = ZOOM_IMAGE_DOWNSCALE,
	.win_strategy = ZOOM_WIN_FIXED,
	.full_screen = 0,
	.exif_autorotate = 1,
	.max_win_w = 1024,
	.max_win_h = 768,

	.font_path = NULL,
	.font_height = 12,
};

static int set_zoom_strategy(struct cfg_opt *self, unsigned int lineno)
{
	switch (self->opt) {
	case 'w':
		switch (self->val[0]) {
		case 'r':
		case 'R':
			config.win_strategy = ZOOM_WIN_RESIZABLE;
			return 0;
		case 'f':
		case 'F':
			config.win_strategy = ZOOM_WIN_FIXED;
			return 0;
		}
	break;
	case 'z':
		switch (self->val[0]) {
		case 'n':
		case 'N':
			config.zoom_strategy = 0;
			return 0;
		case 'u':
		case 'U':
			config.zoom_strategy = ZOOM_IMAGE_UPSCALE;
			return 0;
		case 'd':
		case 'D':
			config.zoom_strategy = ZOOM_IMAGE_DOWNSCALE;
			return 0;
		case 'b':
		case 'B':
			config.zoom_strategy = ZOOM_IMAGE_UPSCALE |
			                       ZOOM_IMAGE_DOWNSCALE;
			return 0;
		}
	break;
	}

	fprintf(stderr, "ERROR: %u: Invalid zoom strategy '%s'\n",
	        lineno, self->val);
	return 1;
}

static int set_win_max_size(struct cfg_opt *self, unsigned int lineno)
{
	unsigned int w, h;

	if (sscanf(self->val, "%ux%u", &w, &h) != 2) {
		fprintf(stderr, "ERROR: %u: Invalid max window size '%s'\n",
		        lineno, self->val);
		return 1;
	}

	config.max_win_w = w;
	config.max_win_h = h;
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
	case 'd':
		config.floyd_steinberg = 1;
	break;
	case 'f':
		config.full_screen = 1;
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
	case 'r':
		config.exif_autorotate = 0;
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
	config.emul_type = gp_pixel_type_by_name(optarg);

	if (config.emul_type == GP_PIXEL_UNKNOWN) {
		fprintf(stderr, "ERROR: %u: Invalid pixel type '%s'\n",
		        lineno, self->val);
		return 1;
	}

	return 0;
}

int load_font_face(gp_text_style *style, const char *path, unsigned int height,
                   unsigned int lineno)
{
	gp_font_face *font;
	static gp_font_face *old_font = NULL;

	font = gp_font_face_load(path, 0, height);

	if (!font) {
		fprintf(stderr, "ERROR: %u: Failed to load font '%s'\n",
		        lineno, path);
		return 1;
	}

	gp_font_face_free(old_font);
	style->font = old_font = font;

	return 0;
}

static int set_font(struct cfg_opt *self, unsigned int lineno)
{
	static gp_text_style style = {NULL, 0, 0, 1, 1, 0};

	if (load_font_face(&style, self->val, config.font_height, lineno))
		return 1;

	free(config.font_path);
	config.font_path = strdup(self->val);
	config.style = &style;

	return 0;
}

static int set_font_height(struct cfg_opt *self, unsigned int lineno)
{
	int height = atoi(self->val);

	if (height <= 0) {
		fprintf(stderr, "ERROR: %u: Wrong font height '%s'\n",
		        lineno, self->val);
		return 1;
	}

	config.font_height = height;

	if (config.style) {
		return load_font_face(config.style, config.font_path,
		                      config.font_height, lineno);
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
	 .key = "FontPath",
	 .opt_long = "font-path",
	 .opt_has_value = 1,
	 .set = set_font,
	 .help = "Path to TTF font to be used in GUI",
	},
	{.name_space = "Gui",
	 .key = "FontHeight",
	 .opt_long = "font-height",
	 .opt_has_value = 1,
	 .set = set_font_height,
	 .help = "TTF font height in pixels",
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
	 .key = "Dithering",
	 .opt = 'd',
	 .opt_long = "dithering",
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
	 .key = "DisableExifAutorotate",
	 .opt = 'r',
	 .opt_long = "disable_exif_autorotate",
	 .opt_has_value = 0,
	 .set = set_opt,
	 .help = "Disables automatic rotation by EXIF",
	},
	{.name_space = "Gui",
	 .key = "FullScreen",
	 .opt = 'f',
	 .opt_long = "full-screen",
	 .opt_has_value = 0,
	 .set = set_opt,
	 .help = "Start fullscreen.",
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
	 .key = "WindowSize",
	 .opt = 'w',
	 .opt_long = "window-size",
	 .opt_has_value = 1,
	 .set = set_zoom_strategy,
	 .help = "Window size, resizeable (-wr) or fixed (-wf)",
	},
	{.name_space = "Zoom",
	 .key = "ZoomStrategy",
	 .opt = 'z',
	 .opt_long = "zoom-strategy",
	 .opt_has_value = 1,
	 .set = set_zoom_strategy,
	 .help = "Zoom strategy, none (-zn), upscale (-zu), "
	         "downscale (-zd) or both (-zb)",
	},
	{.name_space = "Zoom",
	 .key = "MaxWinSize",
	 .opt = 'm',
	 .opt_long = "max-win-size",
	 .opt_has_value = 1,
	 .set = set_win_max_size,
	 .help = "Window maximal size, 800x600 for example",
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
