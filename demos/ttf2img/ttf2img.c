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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Uses gfxprim to render small image with defined string.

  */

#include <GP.h>

static const char help[] = {
	"usage: ttf2img -f font.ttf -i file.png -s string [-d debug_level]\n",
};

static void print_help(int i)
{
	fputs(help, stderr);
	exit(i);
}

int main(int argc, char *argv[])
{
	const char *font_path = NULL;
	const char *img_path = NULL;
	const char *string   = "Foo Bar!";
	int opt, debug_level = 0;
	int img_w = 400, img_h = 100;

	while ((opt = getopt(argc, argv, "d:f:i:s:")) != -1) {
		switch (opt) {
		case 'f':
			font_path = optarg;
		break;
		case 'i':
			img_path = optarg;
		break;
		case 'd':
			debug_level = atoi(optarg);
		break;
		case 'h':
			print_help(0);
		break;
		case 's':
			string = optarg;
		break;
		default:
			fprintf(stderr, "Invalid paramter '%c'\n", opt);
			print_help(1);
		}
	}

	if (font_path == NULL || img_path == NULL)
		print_help(1);

	GP_SetDebugLevel(debug_level);

	GP_Context *context = GP_ContextAlloc(img_w, img_h, GP_PIXEL_RGB888);

	GP_Pixel black_pixel = GP_ColorToContextPixel(GP_COL_BLACK, context);
	GP_Pixel white_pixel = GP_ColorToContextPixel(GP_COL_WHITE, context);

	GP_Fill(context, white_pixel);

	GP_TextStyle style = GP_DEFAULT_TEXT_STYLE;

	style.font = GP_FontFaceLoad(font_path, 27, 0);

	GP_Text(context, &style, img_w/2, img_h/2, GP_ALIGN_CENTER|GP_VALIGN_CENTER,
	        black_pixel, white_pixel, string);

	GP_SavePNG(context, img_path, NULL);

	GP_ContextFree(context);

	return 0;
}
