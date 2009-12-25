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

#include "GP_pixel.h"

/*
 * Sets a pixel at coordinates (x, y) to the given color.
 * If the coordinates lie outside the surface boundaries or outside its
 * clipping rectangle, the call has no effect.
 */
void GP_SetPixel(SDL_Surface *surf, long color, int x, int y)
{
	int bytes_per_pixel = surf->format->BytesPerPixel;

	if (surf == NULL || surf->pixels == NULL)
		return;

	/* Clip coordinates against the clip rectangle of the surface */
	int x1 = surf->clip_rect.x;
	int y1 = surf->clip_rect.y;
	int x2 = surf->clip_rect.x + surf->clip_rect.w;
	int y2 = surf->clip_rect.y + surf->clip_rect.h;

	if (x < x1 || y < y1 || x >= x2 || y >= y2)
		return;

	/* Compute the address of the pixel */
	Uint8 *p = ((Uint8 *) surf->pixels) + y * surf->pitch + x * bytes_per_pixel;

	switch(bytes_per_pixel) {
	case 1:
		*p = (Uint8) color;
		break;
	
	case 2:
		*(Uint16 *)p = (Uint16) color;
		break;

	case 3:
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (color >> 16) & 0xff;
			p[1] = (color >> 8) & 0xff;
			p[2] = color & 0xff;
		} else {
			p[0] = color & 0xff;
			p[1] = (color >> 8) & 0xff;
			p[2] = (color >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32 *)p = (Uint32) color;
		break;
	}
}

/*
 * Specializations of GP_SetPixel() for surfaces of known bit depth:
 * GP_SetPixel_8bpp(), GP_SetPixel_16bpp(), GP_SetPixel_24bpp(),
 * GP_SetPixel_32bpp().
 */

void GP_SetPixel_8bpp(SDL_Surface *surf, long color, int x, int y)
{
	if (surf == NULL || surf->pixels == NULL)
		return;

	int x1 = surf->clip_rect.x;
	int y1 = surf->clip_rect.y;
	int x2 = surf->clip_rect.x + surf->clip_rect.w;
	int y2 = surf->clip_rect.y + surf->clip_rect.h;

	if (x < x1 || y < y1 || x >= x2 || y >= y2)
		return;

	Uint8 *p = ((Uint8 *) surf->pixels) + y * surf->pitch + x;

	*p = (Uint8) color;
}

void GP_SetPixel_16bpp(SDL_Surface *surf, long color, int x, int y)
{
	if (surf == NULL || surf->pixels == NULL)
		return;

	int x1 = surf->clip_rect.x;
	int y1 = surf->clip_rect.y;
	int x2 = surf->clip_rect.x + surf->clip_rect.w;
	int y2 = surf->clip_rect.y + surf->clip_rect.h;

	if (x < x1 || y < y1 || x >= x2 || y >= y2)
		return;

	Uint8 *p = ((Uint8 *) surf->pixels) + y * surf->pitch + 2*x;

	*(Uint16 *)p = (Uint16) color;
}

void GP_SetPixel_24bpp(SDL_Surface *surf, long color, int x, int y)
{
	if (surf == NULL || surf->pixels == NULL)
		return;

	int x1 = surf->clip_rect.x;
	int y1 = surf->clip_rect.y;
	int x2 = surf->clip_rect.x + surf->clip_rect.w;
	int y2 = surf->clip_rect.y + surf->clip_rect.h;

	if (x < x1 || y < y1 || x >= x2 || y >= y2)
		return;

	Uint8 *p = ((Uint8 *) surf->pixels) + y * surf->pitch + 3*x;

	if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
		p[0] = (color >> 16) & 0xff;
		p[1] = (color >> 8) & 0xff;
		p[2] = color & 0xff;
	} else {
		p[0] = color & 0xff;
		p[1] = (color >> 8) & 0xff;
		p[2] = (color >> 16) & 0xff;
	}
}

void GP_SetPixel_32bpp(SDL_Surface *surf, long color, int x, int y)
{
	if (surf == NULL || surf->pixels == NULL)
		return;

	int x1 = surf->clip_rect.x;
	int y1 = surf->clip_rect.y;
	int x2 = surf->clip_rect.x + surf->clip_rect.w;
	int y2 = surf->clip_rect.y + surf->clip_rect.h;

	if (x < x1 || y < y1 || x >= x2 || y >= y2)
		return;

	Uint8 *p = ((Uint8 *) surf->pixels) + y * surf->pitch + 4*x;

	*(Uint32 *)p = (Uint32) color;
}

