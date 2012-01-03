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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Gamma correction.

 */

#ifndef CORE_GP_GAMMA_CORRECTION_H
#define CORE_GP_GAMMA_CORRECTION_H

#include <stdint.h>
#include <math.h>

#define GP_GAMMA 2.2

/*
 * Coverts linear 0 255 value into 0 255 gama value.
 *
 * (this is used for Anti Aliased gfx primitives.
 */
static inline uint8_t GP_GammaToLinear(uint8_t val)
{
	return pow(1.00 * val/255, 1/GP_GAMMA) * 255 + 0.5;
}

#endif /* CORE_GP_GAMMA_CORRECTION_H */
