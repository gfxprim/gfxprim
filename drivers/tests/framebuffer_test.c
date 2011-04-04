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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

 /*

   Framebuffer test. 
   
   1. Draw to the framebuffer
   2. Sleep
   3. Exit

  */

#include <math.h>

#include <GP.h>
#include <GP_Framebuffer.h>

static void draw(GP_Context *context, int x, int y, int l)
{
	int x2, y2, x3, y3;

	GP_Pixel red, blue, green;
	
	GP_RGBToPixel(context, 255,   0,   0, &red);
	GP_RGBToPixel(context,   0,   0, 255, &blue);
	GP_RGBToPixel(context,   0, 255,   0, &green);

	x2 = x + l/2;
	y2 = y + sqrt(2)/2 * l;
	x3 = x - l/2;
	y3 = y2;

	GP_FillTriangle(context,  x,  y, x2, y2, x + l, y, red); 
	GP_FillTriangle(context,  x,  y, x3, y3, x - l, y, green); 
	GP_FillTriangle(context, x2, y2, x3, y3, x, y + sqrt(2) * l, blue); 
}

int main(void)
{
	GP_Framebuffer *fb = GP_FramebufferInit("/dev/fb0");
	GP_TextStyle style;

	if (fb == NULL)
		return 1;
	
	GP_DefaultTextStyle(&style);

	style.pixel_xspace = 2;
	style.pixel_ymul   = 2;

	GP_Pixel gray, black;

	GP_RGBToPixel(&fb->context, 200, 200, 200, &gray);
	GP_RGBToPixel(&fb->context,   0,   0,   0, &black);
	
	const char *text = "Framebuffer test";

	GP_Fill(&fb->context, gray);
	GP_Line(&fb->context, 0, 0, fb->context.w, fb->context.h, black);
	GP_Line(&fb->context, 0, fb->context.h, fb->context.w, 0, black);
	GP_Text(&fb->context, &style,
	        (fb->context.w - GP_TextWidth(&style, text))/2,
		16, GP_ALIGN_RIGHT|GP_VALIGN_BELOW, text, black);


	draw(&fb->context, fb->context.w / 2, 2.00 * fb->context.h / 3, 60);

	sleep(10);

	GP_FramebufferExit(fb);

	return 0;
}
