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
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>                  *
 *                                                                           *
 *****************************************************************************/

#include "GP_Pixel.h"
#include "GP_Pixel_Access.h"
#include "GP_Context.h"

/*
void GP_Blit(const GP_Context *c1, int x1, int y1, int w, int h,
             GP_Context *c2, int x2, int y2)
{
      // Ultimate TODO: effective processing 
      GP_Blit_Naive(c1, x1, y1, w, h, c2, x2, y2);
}
*/

// TODO(gavento, czech) Plan:
// GP_Blit_Naive - Zadne rotovani a tak, jen Get/PutPixel a konverze A->RGBA8888->B
// GP_Blit_Simple - S rotovanim, makrovy Get/PutPixel, mozna optimalizace na radky, chytrejsi konverze (ale porad univ.)
// GP_Blit_Simple_xBPP - S rotovanim, makrovy Get/PutPixel
// GP_Blit_xBPP - Optimalizovane, muze volat GP_Blit_Simple_xBPP pro divne pripady
// GP_Blit - Vola GP_Blit_xBPP (stejny typ) nebo GP_Blit_Simple (jine typy), pripadne optimalizovat

/*
void GP_Blit_Naive(const GP_Context *c1, int x1, int y1, int w, int h,
                   GP_Context *c2, int x2, int y2)
{
	GP_TRANSFORM_BLIT(c1, x1, y1, w, h, c2, x2, y2);
	// TODO: Cipping?
	GP_Blit_Naive_Raw(c1, x1, y1, w, h, c2, x2, y2);
}
*/

/*
 * Very naive blit, no optimalizations whatsoever - keep it that way.
 * Used as a reference for testing and such. Aaand ultimate fallback.
 */
void GP_Blit_Naive(const GP_Context *c1, int x1, int y1, int w, int h,
                   GP_Context *c2, int x2, int y2)
{
	GP_CHECK(x1 >= 0);
	GP_CHECK(y1 >= 0);
	GP_CHECK(w >= 0);
	GP_CHECK(h >= 0);
	GP_CHECK(x1 + w <= GP_ContextW(c1));
	GP_CHECK(y1 + h <= GP_ContextH(c1));
	GP_CHECK(x2 >= 0);
	GP_CHECK(y2 >= 0);
	GP_CHECK(x2 + w <= GP_ContextW(c2));
	GP_CHECK(y2 + h <= GP_ContextH(c2));

	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++) {
			GP_Pixel p = GP_GetPixel(c1, x1 + i, y1 + j);
			GP_PutPixel(c2, x2 + i, y2 + j, p);
		}
}
