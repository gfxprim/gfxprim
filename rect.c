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
 * Draws a rectangle from (x0, y0) to (x1, y1).
 */
void GP_Rect(SDL_Surface * surf, long color, int x0, int y0, int x1, int y1)
{
	GP_Line(surf, color, x0, y0, x1, y0);
	GP_Line(surf, color, x0, y1, x1, y1);
	GP_Line(surf, color, x0, y0, x0, y1);
	GP_Line(surf, color, x1, y0, x1, y1);
}

/*
 * Draws a solid filled rectangle from (x0, y0) to (x1, y1).
 */
void GP_FillRect(SDL_Surface * surf, long color, int x0, int y0, int x1, int y1)
{
	int top, left, width, height;

	if (y0 <= y1) {
		top = y0;
		height = y1 - y0;
	}
	else {
		top = y1;
		height = y0 - y1;
	}

	if (x0 <= x1) {
		left = x0;
		width = x1 - x0;
	}
	else {
		left = x1;
		width = x0 - x1;
	}

	SDL_Rect rect = { x0, y0, width + 1, height + 1 };
	SDL_FillRect(surf, &rect, color);
}

