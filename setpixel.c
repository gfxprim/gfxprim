/*
 * This file is part of the gfxprim library.
 *
 * Copyright (c) 2009 Jiri "BlueBear" Dluhos <jiri.bluebear.dluhos@gmail.com>,
 *                    Cyril Hrubis (Metan) <metan@ucw.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "GP.h"

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

