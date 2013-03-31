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

 /*

   Simple backend example.

  */

#include <GP.h>

static void redraw(GP_Backend *self)
{
	GP_Context *context = self->context;
	GP_Pixel white_pixel, black_pixel;

	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, context);

	GP_Fill(context, black_pixel);
	GP_Line(context, 0, 0, context->w - 1, context->h - 1, white_pixel);
	GP_Line(context, 0, context->h - 1, context->w - 1, 0, white_pixel);

	/* Update the backend screen */
	GP_BackendFlip(self);
}

int main(int argc, char *argv[])
{
	GP_Backend *backend;
	const char *backend_opts = "X11:100x100";
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

	/* Turn on debug messages */
	GP_SetDebugLevel(10);

	backend = GP_BackendInit(backend_opts, "Backend Example", stderr);

	if (backend == NULL) {
		fprintf(stderr, "Failed to initialize backend\n");
		return 1;
	}

	redraw(backend);

	/* Handle events */
	for (;;) {
		GP_Event ev;

		GP_BackendWaitEvent(backend, &ev);

		GP_EventDump(&ev);
		
		switch (ev.type) {
		case GP_EV_KEY:
			switch (ev.val.val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				GP_BackendExit(backend);
				return 0;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev.code) {
			case GP_EV_SYS_RESIZE:
				GP_BackendResizeAck(backend);
				redraw(backend);
			break;
			case GP_EV_SYS_QUIT:
				GP_BackendExit(backend);
				return 0;
			break;
			}
		break;
		}
	}

	GP_BackendExit(backend);

	return 0;
}
