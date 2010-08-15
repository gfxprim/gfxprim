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

#include <GP.h>
#include <GP_Framebuffer.h>

static struct GP_TextStyle style = {
	.font = &GP_default_console_font,
	.pixel_xmul = 2,
	.pixel_ymul = 2,
	.pixel_xspace = 2,
	.pixel_yspace = 2
};

int main(void)
{
	GP_Framebuffer *fb = GP_FramebufferInit("/dev/fb0");
	GP_Color White = GP_COLNAME_PACK(GP_COL_WHITE);
	GP_Color Gray  = GP_COLNAME_PACK(GP_COL_GRAY_DARK);
	GP_Color Gray2 = GP_COLNAME_PACK(GP_COL_GRAY_LIGHT);
	const char *text = "Framebuffer test";

	if (fb == NULL)
		return 1;

	GP_Fill(&fb->context, Gray);
	GP_Line(&fb->context, 0, 0, fb->context.w, fb->context.h, White);
	GP_Line(&fb->context, 0, fb->context.h, fb->context.w, 0, White);
	GP_Text(&fb->context, &style,
	        (fb->context.w - GP_TextWidth(&style, text))/2,
		16, text, Gray2);

	sleep(10);

	GP_FramebufferExit(fb);

	return 0;
}
