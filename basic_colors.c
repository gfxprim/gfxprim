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

