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

   Simple virtual backend test.

   Virtual backned allows you to test interactively pixel types that your
   hardware/xserver doesn't support.

  */

#include <GP.h>

int main(int argc, char *argv[])
{
	GP_Backend *backend;
	GP_Context *context;
	GP_Pixel white_pixel, black_pixel, red_pixel, blue_pixel, green_pixel;
	const char *backend_opts = "X11:350x350";
	int opt;
	GP_PixelType emul_type = GP_PIXEL_UNKNOWN;

	while ((opt = getopt(argc, argv, "b:h:p:")) != -1) {
		switch (opt) {
		case 'b':
			backend_opts = optarg;
		break;
		case 'p':
			emul_type = GP_PixelTypeByName(optarg);

			if (emul_type == GP_PIXEL_UNKNOWN) {
				fprintf(stderr, "Invalid pixel type '%s'\n", optarg);
				return 1;
                        }
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

	backend = GP_BackendInit(backend_opts, "Virtual Backend Example", stderr);

	if (emul_type != GP_PIXEL_UNKNOWN) {
		GP_Backend *emul;
		
		/* 
		 * Create an emulated backend on the top of real backend.
		 * 
		 * The GP_BACKEND_CALL_EXIT says that when calling exit on
		 * emulated backend, the real backend exit will be called as
		 * well.
		 */
		emul = GP_BackendVirtualInit(backend, emul_type, GP_BACKEND_CALL_EXIT);

		if (emul == NULL) {
			fprintf(stderr, "Failed to create Virtual Backend\n");
			GP_BackendExit(backend);
			return 1;
		}

		/* Once created virtual backend behaves exactly like a real one */
		backend = emul;
	}

	context = backend->context;

	GP_EventSetScreenSize(context->w, context->h);
	
	/* Now draw some testing patters */
	black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);
	white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, context);
	red_pixel   = GP_ColorToContextPixel(GP_COL_RED, context);
	blue_pixel  = GP_ColorToContextPixel(GP_COL_BLUE, context);
	green_pixel = GP_ColorToContextPixel(GP_COL_GREEN, context);

	GP_Fill(context, white_pixel);
	
	unsigned int i, j;
	for (i = 0; i < 40; i++) {
		GP_HLineXYW(context, 0, i, i, black_pixel);
		GP_HLineXYW(context, 1, i + 40, i, black_pixel);
		GP_HLineXYW(context, 2, i + 80, i, black_pixel);
		GP_HLineXYW(context, 3, i + 120, i, black_pixel);
		GP_HLineXYW(context, 4, i + 160, i, black_pixel);
		GP_HLineXYW(context, 5, i + 200, i, black_pixel);
		GP_HLineXYW(context, 6, i + 240, i, black_pixel);
		GP_HLineXYW(context, 7, i + 280, i, black_pixel);
	}

	for (i = 0; i < 256; i++) {
		for (j = 0; j < 256; j++) {
			uint8_t val = 1.00 * sqrt(i*i + j*j)/sqrt(2) + 0.5;

			GP_Pixel pix = GP_RGBToContextPixel(i, j, val, context);
			GP_PutPixel(context, i + 60, j + 10, pix);
		}
	}

	GP_Text(context, NULL, 60, 270, GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
	        black_pixel, white_pixel, "Lorem Ipsum dolor sit...");
	
	GP_Text(context, NULL, 60, 290, GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
	        red_pixel, white_pixel, "Lorem Ipsum dolor sit...");
	
	GP_Text(context, NULL, 60, 310, GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
	        green_pixel, white_pixel, "Lorem Ipsum dolor sit...");

	GP_Text(context, NULL, 60, 330, GP_VALIGN_BELOW|GP_ALIGN_RIGHT,
	        blue_pixel, white_pixel, "Lorem Ipsum dolor sit...");
	
	/* Update the backend screen */
	GP_BackendFlip(backend);

	for (;;) {
		if (backend->Poll)
			GP_BackendPoll(backend);

		usleep(1000);

		/* Read and parse events */
		GP_Event ev;

		while (GP_BackendEventGet(backend, &ev)) {

			GP_EventDump(&ev);
			
			switch (ev.type) {
			case GP_EV_KEY:
				switch (ev.val.key.key) {
				case GP_KEY_ESC:
				case GP_KEY_Q:
					GP_BackendExit(backend);
					return 0;
				break;
				}
			}
		}
	}

	GP_BackendExit(backend);

	return 0;
}
