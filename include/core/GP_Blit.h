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
 * Copyright (C) 2011      Cyril Hrubis    <metan@ucw.cz>                    *
 *                                                                           *
 *****************************************************************************/

#ifndef CORE_GP_BLIT_H
#define CORE_GP_BLIT_H

/* Generated header */
#include "GP_Blit.gen.h"

void GP_Blit(const GP_Context *c1, GP_Coord x1, GP_Coord y1,
             GP_Size w, GP_Size h, GP_Context *c2, GP_Coord x2, GP_Coord y2);


/*
 * Very naive blit, no optimalizations whatsoever - keep it that way.
 * Used as a reference for testing and such. Aaand ultimate fallback.
 * GP_CHECKS for clipping and size (for testing)
 */
void GP_Blit_Naive(const GP_Context *c1, GP_Coord x1, GP_Coord y1, GP_Size w, GP_Size h,
                   GP_Context *c2, GP_Coord x2, GP_Coord y2);

/* 
 * Similar in purpose to GP_Blit_Naive, but operates on raw coordinates.
 * Does no range checking.
 */
/*
void GP_Blit_Naive_Raw(const GP_Context *c1, int x1, int y1, int w, int h,
                   GP_Context *c2, int x2, int y2);
*/
#endif // CORE_GP_BLIT_H
