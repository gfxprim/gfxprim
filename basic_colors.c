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

#include "GP_colors.h"

/*
 * Fills the specified array with surface-compatible values
 * for the basic colors.
 * The array must accomodate at least GP_BASIC_COLOR_COUNT indices.
 */
void GP_LoadBasicColors(SDL_Surface *surf, long *colors)
{
	colors[GP_BLACK] 	= SDL_MapRGB(surf->format,   0,   0,   0);
	colors[GP_RED] 		= SDL_MapRGB(surf->format, 255,   0,   0);
	colors[GP_GREEN] 	= SDL_MapRGB(surf->format,   0, 255,   0);
	colors[GP_BLUE] 	= SDL_MapRGB(surf->format,   0,   0, 255);
	colors[GP_YELLOW] 	= SDL_MapRGB(surf->format, 255, 255,   0);
	colors[GP_CYAN] 	= SDL_MapRGB(surf->format,   0, 255, 255);
	colors[GP_MAGENTA] 	= SDL_MapRGB(surf->format, 255,   0, 255);
	colors[GP_WHITE] 	= SDL_MapRGB(surf->format, 255, 255, 255);
	colors[GP_MID_RED] 	= SDL_MapRGB(surf->format, 127,   0,   0);
	colors[GP_MID_GREEN] 	= SDL_MapRGB(surf->format,   0, 127,   0);
	colors[GP_MID_BLUE] 	= SDL_MapRGB(surf->format,   0,   0, 127);
	colors[GP_MID_YELLOW] 	= SDL_MapRGB(surf->format, 127, 127,   0);
	colors[GP_MID_CYAN] 	= SDL_MapRGB(surf->format,   0, 127, 127);
	colors[GP_MID_MAGENTA] 	= SDL_MapRGB(surf->format, 127,   0, 127);
	colors[GP_MID_WHITE] 	= SDL_MapRGB(surf->format, 127, 127, 127);
	colors[GP_DARK_RED]     = SDL_MapRGB(surf->format,  63,   0,   0);
	colors[GP_DARK_GREEN]   = SDL_MapRGB(surf->format,   0,  63,   0);
	colors[GP_DARK_BLUE]	= SDL_MapRGB(surf->format,   0,   0,  63);
	colors[GP_DARK_YELLOW]	= SDL_MapRGB(surf->format,  63,  63,   0);
	colors[GP_DARK_CYAN]	= SDL_MapRGB(surf->format,   0,  63,  63);
	colors[GP_DARK_MAGENTA]	= SDL_MapRGB(surf->format,  63,   0,  63);
	colors[GP_DARK_WHITE]   = SDL_MapRGB(surf->format,  63,  63,  63);
}

