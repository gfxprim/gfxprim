// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <gfxprim.h>

#include "spiv_config.h"
#include "spiv_help.h"

struct key_help {
	const char *keys;
	const char *desc;
};

#define KEYS_MAX "13"

static struct key_help help_keys[] = {
	{"Esc, Enter, Q", "Quit spiv"},
	{"Space", "Move to the next image"},
	{"BackSpace", "Move to the prev image"},
	{"PgDown", "Move to the start of directory"},
	{"PgUp", "Move to the end of directory"},
	{"Home", "Move to the first image"},
	{"End", "Move to the last image"},
	{"R", "Rotate by 90 degrees clockwise"},
	{"E", "Rotate by 90 degrees counterclockwise"},
	{"W", "Toggle fixed, resizable window"},
	{"D", "Turn on/off downscale when image is larger than win"},
	{"U", "Turn on/off upscale when image is smaller than win"},
	{"H", "Show help"},
	{"I", "Toggle show info box"},
	{"P", "Toggle show progress"},
	{"S", "Start/stop slideshow"},
	{"", ""},
	{"F1-F10", "Execute action 1 - 10"},
	{"", ""},
	{"<, KP Minus", "Zoom out by 50% (by 10% with Shift)"},
	{">, KP Plus",  "Zoom in by 50% (by 10% with Shift)"},
	{"1", "Resize to the image size"},
	{"2", "Resize to a half of the image size"},
	{"3", "Resize to one third of the image size"},
	{"9", "Resize to one ninth of the image size"},
	{"...", ""},
	{"0", "Resize to one tenth of the image size"},
	{"Shift 2", "Resize twice of the image size"},
	{"Shift 3", "Resize three times of the image size"},
	{"...", ""},
	{"Up", "Move image by 10px up (by 1 with Shift)"},
	{"Down", "Move image by 10px down (by 1 with Shift)"},
	{"Left", "Move image by 10px left (by 1 with Shift)"},
	{"Right", "Move image by 10px right (by 1 with Shift)"},
        {"", ""},
	{"]", "Change to next resampling method"},
	{"[", "Change to prev resampling method"},
	{"L", "Toggle low pass filter"},
	{"C", "Drop image cache"},
};

static const int help_keys_len = sizeof(help_keys) / sizeof(*help_keys);

struct examples {
	const char *example;
	const char *desc;
};

static const struct examples examples[] = {
	{"spiv *.jpg",
	 "Shows all jpeg images in current directory"},
	{"spiv images.zip",
	 "Shows all images stored in zip file"},
	{"spiv .",
	 "Shows all loadable images in current directory"},
	{"spiv -s 5 vacation/",
	 "Runs slideshow with 5 second delay"},
	{"spiv -1 'cp %F sorted' images/",
	 "Copies currently loaded image into directory 'sorted/' on pressing F1"},
	{"spiv -e G1 -d images/",
	 "Emulates 1-bit Grayscale display and turns on Floyd-Steinberg dithering"},
	{"spiv -b 'X11:use_root' -t 10 images/",
	 "Runs slideshow using X root window as backend window"},
	{"spiv -b 'X11:create_root' -t 10 images/",
	 "Same as above but works in KDE\n"}
};

static const int examples_len = sizeof(examples) / sizeof(*examples);

struct actions {
	const char modifier;
	const char *desc;
};

static struct actions actions[] = {
	{'f', "Path to current image"},
	{'F', "Shell escaped path to current image"},
	{'n', "Current image filename without extension"},
	{'N', "Shell escaped image filename without extension"},
	{'e', "Current image file extension"},
};

static const int actions_len = sizeof(actions) / sizeof(*actions);

void print_help(void)
{
	int i;

	printf("Usage: spiv [opts] images or dirs with images\n");
	spiv_config_print_help();

	printf(" Action shell command modifiers:\n");

	for (i = 0; i < actions_len; i++)
		printf("  %%%c %s\n", actions[i].modifier, actions[i].desc);

	printf("\n");

	printf("Keyboard controls:\n\n");

	for (i = 0; i < help_keys_len; i++) {
		if (help_keys[i].desc[0] == '\0') {
			printf(" %s\n", help_keys[i].keys);
		} else {
			printf(" %-"KEYS_MAX"s - %s\n",
			       help_keys[i].keys, help_keys[i].desc);
		}
	}

	puts("");

	printf("Example usage:\n\n");

	for (i = 0; i < examples_len; i++)
		printf("%s\n\t%s\n", examples[i].example, examples[i].desc);
}

const char *man_head =
	".TH spiv 1 2013 GFXprim \"Simple yet Powerful Image Viewer\"\n\n"
	".SH NAME\n"
	"spiv \\- Simple yet Powerful Image Viewer\n"
	".SH SYNOPSIS\n"
	".B spiv\n"
	"[options] images|dirs\n"
	".SH DESCRIPTION\n"
	".B spiv\n"
	"is a fast, lightweight and minimalistic image viewer build on the\n"
	"top of the GFXprim library.\n"
	".PP\n"
	"Spiv supports wide range of image formats, currently supported are\n"
	"JPEG, PNG, GIF, BMP, TIFF, PSP, PSD, PNM, PCX, JPEG2000 and CBZ\n"
	"(as well general ZIP archives with images), and more will come in\n"
	"the near future.\n"
	".PP\n"
	"Spiv supports variety of video backends (via GFXprim backends)\n"
	"currently these are X11, Linux Framebuffer, SDL and AAlib. Spiv also\n"
	"supports wide range of backend pixel types from 1bit Grayscale to 32bit RGB\n"
	"with optional Floyd-Steinberg dithering (even, for example, from RGB888 to RGB565).\n"
	".PP\n"
	"Spiv implements feh-like image actions, which are short shell scripts with\n"
	"printf-like modifiers.\n"
	"See\n.B ACTIONS\nbelow for further information.\n";

static const char *man_tail =
	".SH BUGS\n"
	"Bugs happen. If you find one, report it on the GFXprim mailing list at\n"
	".I gfxprim@ucw.cz\n"
	".SH AUTHORS\n"
	"Spiv is developed by Cyril Hrubis <metan@ucw.cz>\n"
	".PP\nGFXprim was/is developed by:\n"
	".PP\n.nf\nCyril Hrubis <metan@ucw.cz>\n"
	".nf\nJiri \"BlueBear\" Dluhos <jiri.bluebear.dluhos@gmail.com>\n"
	".nf\nTomas Gavenciak <gavento@ucw.cz>\n";

static const char *actions_help =
	".SH ACTIONS\n"
	"Actions are short shell scripts with printf-like modifiers, the \n"
	"modifiers are substituted to current image path, name, etc. and executed\n"
	"by pressing function keys).\n"
	".PP\n"
	"Actions could be set via command line parameters or written into the\n"
	"configuration file and support following modifiers:\n";

void print_man(void)
{
	int i;

	puts(man_head);

	printf(".SH KEYBOARD CONTROL\n");

	for (i = 0; i < help_keys_len; i++) {
		if (help_keys[i].desc[0] != '\0') {
			printf(".IP \"%s\"\n", help_keys[i].keys);
			printf("%s\n", help_keys[i].desc);
		}
	}

	spiv_config_print_man();

	puts(".PP\nConfiguration is loaded from /etc/spiv.conf");
	puts("then ~/.spiv and overriden by command line parameters.\n");

	puts(actions_help);

	for (i = 0; i < actions_len; i++)
		printf(".PP\n.B %%%c\n%s\n", actions[i].modifier, actions[i].desc);

	puts(".SH EXAMPLES");

	for (i = 0; i < examples_len; i++)
		printf(".PP\n.B %s\n.nf\n%s\n\n", examples[i].desc, examples[i].example);

	puts(man_tail);
}

static int last_line;

static int redraw_help(gp_backend *backend, unsigned int loff, gp_coord xoff)
{
	gp_pixmap *c = backend->pixmap;
	gp_pixel black = gp_rgb_to_pixmap_pixel(0x00, 0x00, 0x00, c);
	gp_pixel white = gp_rgb_to_pixmap_pixel(0xff, 0xff, 0xff, c);
	int i;

	int spacing = gp_text_height(config.style)/10 + 2;
	int height = gp_text_height(config.style);

	gp_fill(c, black);

	gp_print(c, config.style, 20 + 10 * xoff, 2, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
	         white, black, "%s", "Keyboard Controls:");

	unsigned int max = 0;

	for (i = 0; i < help_keys_len; i++)
		max = GP_MAX(max, gp_text_wbbox(config.style, help_keys[i].keys));

	for (i = loff; i < help_keys_len; i++) {
		gp_coord h = spacing + (i - loff + 1) * (height + spacing);

		if (h + height + spacing > (gp_coord)c->h) {
			last_line = 0;
			goto out;
		}

		gp_print(c, config.style, 20 + 10 * xoff, h, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
		         white, black, "%s", help_keys[i].keys);
		gp_print(c, config.style, 20 + 10 * xoff + max, h, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
		         white, black, "  -  %s", help_keys[i].desc);
		last_line = 1;
	}

out:
	gp_backend_flip(backend);
	return i;
}

void draw_help(gp_backend *backend)
{
	int loff = 0, last, xoff = 0;

	last = redraw_help(backend, loff, xoff);

	for (;;) {
		gp_event *ev;

		while ((ev = gp_backend_ev_wait(backend))) {
			switch (ev->type) {
			case GP_EV_KEY:
				if (ev->code != GP_EV_KEY_DOWN)
					continue;

				switch (ev->key.key) {
				case GP_KEY_DOWN:
					if (last < help_keys_len)
						last = redraw_help(backend, ++loff, xoff);
				break;
				case GP_KEY_UP:
					if (loff > 0)
						last = redraw_help(backend, --loff, xoff);
				break;
				case GP_KEY_LEFT:
					last = redraw_help(backend, loff, --xoff);
				break;
				case GP_KEY_RIGHT:
					last = redraw_help(backend, loff, ++xoff);
				break;
				case GP_KEY_PAGE_DOWN:
					if (last < help_keys_len) {
						if (last_line)
							break;

						loff += last - loff;
						last = redraw_help(backend, loff, xoff);
					}
				break;
				case GP_KEY_PAGE_UP:
					if (loff > 0) {
						loff -= last - loff;
						if (loff < 0)
							loff = 0;
						last = redraw_help(backend, loff, xoff);
					}
				break;
				default:
				return;
				}
			break;
			case GP_EV_SYS:
				switch (ev->code) {
				case GP_EV_SYS_RESIZE:
					gp_backend_resize_ack(backend);
					last = redraw_help(backend, loff, xoff);
				break;
				case GP_EV_SYS_QUIT:
					gp_backend_ev_put_back(backend, ev);
					return;
				}
			}
		}
	}
}
