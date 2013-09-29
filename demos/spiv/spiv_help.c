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
#include <GP.h>

static const char *keys_help[] = {
	"Keyboard control:",
	"",
	"Esc, Enter, Q - quit spiv",
	"",
	"< or KP Minus         - zoom out by 1.5",
	"> or KP Plus          - zoom in by 1.5",
	"R                     - rotate by 90 degrees clockwise",
	"Up, Down, Left, Right - move image by 1px",
	"                        (by 10 with Shift)",
	"",
	"Space     - move to the next image",
	"BackSpace - move to the prev image",
	"PgDown    - move to the start of directory",
	"PgUp      - move to the end of directory",
	"Home      - move to the first image",
	"End       - move to the last image",
	"",
	"I      - toggle show info box",
	"P      - toggle show progress",
	"S      - stop/restart slideshow",
	"",
	"]      - change to next resampling method",
	"[      - change to prev resampling method",
	"        (current method is shown in info box)",
	"L      - toggle low pass filter",
	"D      - drop image cache",
	"H      - toggle help",
	"",
	"F1-F10 - execute action 1 - 10",
	"",
	"1      - resize spiv window to the image size",
	"2      - resize spiv window to the half of the image size",
	"3      - resize spiv window to the third of the image size",
	"...",
	"9      - resize spiv window to the ninth of the image size",
	"0      - resize spiv window to the tenth of the image size",
	"",
	"Shift 2 - resize spiv window twice of the image size",
	"Shift 3 - resize spiv window three times of the image size",
	"...",
};

static const int keys_help_len = sizeof(keys_help) / sizeof(char*);

void print_help(void)
{
	int i;

	printf("Usage: spiv [opts] images or dirs with images\n\n");
	printf(" -I show image info box\n");
	printf(" -P show loading progress\n");
	printf(" -f use floyd-steinberg dithering\n");
	printf(" -s sec slideshow interval in seconds (floating point value)\n");
	printf(" -c turns on bicubic resampling (experimental)\n");
	printf(" -e pixel_type  turns on backend type emulation\n");
	printf("    for example -e G1 sets 1-bit grayscale\n");
	printf(" -r angle  rotate display 90,180 or 270 degrees\n");
	printf(" -z mode\n");
	printf("    -zf zoom is set and modified by user\n");
	printf("    -zw zoom is fixed to window size (currently default)\n");
	printf(" -b pass backend init string to backend init\n");
	printf("    pass -b help for more info\n");
	printf(" -t enable timers\n");
	printf("    if set timers that measure cpu and wall time\n");
	printf("    of certain operations are printed into stdout\n");
	puts("\n");
	printf("Actions:\n\n");
	printf(" -1 'cmd' sets first action\n");
	printf(" ...\n");
	printf(" -9 'cmd' sets ninth action\n");
	printf(" -0 'cmd' sets tenth action\n");
	puts("");
	printf(" actions are shell commands with following modifiers:\n");
	printf("  %%f path to current image\n");
	printf("  %%F shell escaped path to current image\n");
	printf("  %%n current image filename without extension\n");
	printf("  %%N shell escaped image filename without extension\n");
	printf("  %%e current image file extension\n");
	puts("\n");

	for (i = 0; i < keys_help_len; i++)
		puts(keys_help[i]);

	puts("");

	printf("Some cool options to try:\n\n");
	printf("spiv -0 'cp %%F sorted' [images]\n");
	printf("\tcopies current image into directory 'sorted/' on F1\n");
	printf("spiv -e G1 -f [images]\n");
	printf("\truns spiv in 1-bit bitmap mode and turns on dithering\n\n");
	printf("spiv -b 'X11:ROOT_WIN' [images]\n");
	printf("\truns spiv using X root window as backend window\n\n");
	printf("spiv -b 'X11:CREATE_ROOT' [images]\n");
	printf("\tSame as abowe but works in KDE\n");

}

static int redraw_help(GP_Backend *backend, unsigned int loff, GP_Coord xoff)
{
	GP_Context *c = backend->context;
	GP_Pixel black = GP_ColorToContextPixel(GP_COL_BLACK, c);
	GP_Pixel white = GP_ColorToContextPixel(GP_COL_WHITE, c);
	int i;

	GP_Fill(c, black);

	for (i = loff; i < keys_help_len; i++) {
		GP_Coord h = 2 + (i - loff) * 15;

		if (h + 2 >= (GP_Coord)c->h)
			goto out;

		GP_Print(c, NULL, 20 + 10 * xoff, h, GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM,
		         white, black, "%s", keys_help[i]);
	}

out:
	GP_BackendFlip(backend);
	return i;
}

static int max_lines(GP_Backend *backend)
{
	return (backend->context->h - 4) / 15;
}

void draw_help(GP_Backend *backend)
{
	int loff = 0, last, xoff = 0;

	last = redraw_help(backend, loff, xoff);

	for (;;) {
		GP_Event ev;

		while (GP_BackendWaitEvent(backend, &ev)) {
			switch (ev.type) {
			case GP_EV_KEY:
				if (ev.code != GP_EV_KEY_DOWN)
					continue;

				switch (ev.val.key.key) {
				case GP_KEY_DOWN:
					if (last < keys_help_len)
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
					if (last < keys_help_len) {
						if (loff + max_lines(backend) >= keys_help_len)
							break;

						loff += max_lines(backend);

						last = redraw_help(backend, loff, xoff);
					}
				break;
				case GP_KEY_PAGE_UP:
					if (loff > 0) {
						loff -= max_lines(backend);
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
				switch (ev.code) {
				case GP_EV_SYS_RESIZE:
					GP_BackendResizeAck(backend);
					last = redraw_help(backend, loff, xoff);
				break;
				case GP_EV_SYS_QUIT:
					GP_BackendPutEventBack(backend, &ev);
					return;
				}
			}
		}
	}
}
