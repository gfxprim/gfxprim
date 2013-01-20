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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "GP.h"

static GP_Context *win;
static GP_Backend *backend;

static GP_Pixel red, green, white, black;

static void draw_event(GP_Event *ev)
{
	static GP_Size size = 0;

	if (ev->type != GP_EV_KEY)
		return;
	
	int align = GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM;

	GP_TextClear(win, NULL, 20, 20, align, black, size);
	size = GP_Print(win, NULL, 20, 20, align,
	                white, black, "Key=%s",
			GP_EventKeyName(ev->val.key.key));
	
	GP_BackendFlip(backend);
}

static void event_loop(void)
{
	for (;;) {
		GP_BackendWait(backend);
		
		while (GP_EventsQueued()) {
			GP_Event ev;

			GP_EventGet(&ev);
			GP_EventDump(&ev);

			switch (ev.type) {
			case GP_EV_KEY:
				draw_event(&ev);

				switch (ev.val.key.key) {
				case GP_KEY_ESC:
					GP_BackendExit(backend);
					exit(0);
				break;
				case GP_BTN_LEFT:
					GP_HLineXXY(win, ev.cursor_x - 3,
					            ev.cursor_x + 3,
						    ev.cursor_y, red);
					GP_VLineXYY(win, ev.cursor_x,
					            ev.cursor_y - 3,
						    ev.cursor_y + 3, red);
					GP_BackendFlip(backend);
				break;
				default:
				break;
				}
			break;
			case GP_EV_REL:
				switch (ev.code) {
				static int size = 0;
				case GP_EV_REL_POS:
					if (GP_EventGetKey(&ev, GP_BTN_LEFT)) {
						GP_PutPixel(win, ev.cursor_x,
					        	    ev.cursor_y, green);
					}
					int align = GP_ALIGN_RIGHT|GP_VALIGN_BOTTOM;

					GP_TextClear(win, NULL, 20, 40, align,
					             black, size);
					size = GP_Print(win, NULL, 20, 40, align,
					                white, black, "X=%3u Y=%3u dX=%3i dY=%3i",
						        ev.cursor_x, ev.cursor_y,
							ev.val.rel.rx, ev.val.rel.ry);
					GP_BackendFlip(backend);
				break;
				}
			break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	const char *backend_opts = "X11";
	int opt;

	while ((opt = getopt(argc, argv, "b:h")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'h':
			GP_BackendInit(NULL, NULL, stderr);
			return 0;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			return 1;
		}
	}

	backend = GP_BackendInit(backend_opts, "Input Test", stderr);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initalize backend '%s'\n",
		        backend_opts);
		return 1;
	}
	
	win = backend->context;

	red   = GP_ColorToContextPixel(GP_COL_RED, win);
	green = GP_ColorToContextPixel(GP_COL_GREEN, win);
	white = GP_ColorToContextPixel(GP_COL_WHITE, win);
	black = GP_ColorToContextPixel(GP_COL_BLACK, win);

	GP_Fill(win, black);
	GP_BackendFlip(backend);

	GP_EventSetScreenSize(win->w, win->h);

	for (;;) {
		GP_BackendWait(backend);
		event_loop();
	}
}
