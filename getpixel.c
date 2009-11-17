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
 * Reads the color value of a pixel at coordinates (x, y).
 * The clipping rectangle of the surface is ignored.
 * If the coordinates (x, y) lie outside the surface, 0 is returned.
 */
long GP_GetPixel(SDL_Surface * surf, int x, int y)
{
	if (surf == NULL || surf->pixels == NULL)
		return 0;

	/* Clip coordinates against the surface boundary */
	if (x < 0 || y < 0 || x >= surf->w || y >= surf->h)
		return 0;

	/* Compute the address of the pixel */
	int bytes_per_pixel = surf->format->BytesPerPixel;
	Uint8 *p = ((Uint8 *) surf->pixels) + y * surf->pitch + x * bytes_per_pixel;

	switch(bytes_per_pixel) {
	case 1:
		return (long) *p;

	case 2:
		return (long) *((Uint16 *) p);

	case 3:
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return (long)((p[0] << 16) | (p[1] << 8) | p[2]);
		else
			return (long)(p[0] | (p[1] << 8) | (p[2] << 16));
	case 4:
		return (long) *((Uint32 *) p);

	default:
		return 0;
	}
}
