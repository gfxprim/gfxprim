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

static void redraw(struct GP_Context *context)
{
	GP_Pixel white_pixel, black_pixel;
	
	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, context);

	GP_Fill(context, black_pixel);
	GP_Line(context, 0, 0, context->w - 1, context->h - 1, white_pixel);
	GP_Line(context, 0, context->h - 1, context->w - 1, 0, white_pixel);
}

static int ev_loop(struct GP_Backend *backend, const char *name)
{
	GP_Event ev;

	if (backend == NULL)
		return 0;

	while (GP_BackendGetEvent(backend, &ev)) {

		printf("-------------------------- %s\n", name);
		
		GP_EventDump(&ev);
			
		switch (ev.type) {
		case GP_EV_KEY:
			switch (ev.val.val) {
			case GP_KEY_ESC:
			case GP_KEY_Q:
				GP_BackendExit(backend);
				return 1;
			break;
			}
		break;
		case GP_EV_SYS:
			switch (ev.code) {
			case GP_EV_SYS_RESIZE:
				GP_BackendResizeAck(backend);
			break;
			case GP_EV_SYS_QUIT:
				GP_BackendExit(backend);
				return 1;
			break;
			}
		break;
		}
	
		printf("-----------------------------\n");
	}

	return 0;
}

int main(void)
{
	GP_Backend *win_1, *win_2;

	win_1 = GP_BackendX11Init(NULL, 0, 0, 300, 300, "win 1", 0);
	win_2 = GP_BackendX11Init(NULL, 0, 0, 300, 300, "win 2", 0);

	/* Update the backend screen */
	redraw(win_1->context);
	redraw(win_2->context);
	
	GP_BackendFlip(win_1);
	GP_BackendFlip(win_2);

	for (;;) {
		/* 
		 * Wait for backend event.
		 *
		 * Either window is fine as they share connection.
		 */
		GP_Backend *b = win_1 ? win_1 : win_2;

		if (b == NULL)
			return 0;

		GP_BackendWait(b);

		if (ev_loop(win_1, "win 1"))
			win_1 = NULL;

		if (ev_loop(win_2, "win 2"))
			win_2 = NULL;
	}

	GP_BackendExit(win_1);
	GP_BackendExit(win_2);

	return 0;
}
