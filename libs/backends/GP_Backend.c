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

#include "core/GP_Common.h"
#include "core/GP_Transform.h"

#include "backends/GP_Backend.h"

void GP_BackendUpdateRect(GP_Backend *backend,
                          GP_Coord x0, GP_Coord y0,
                          GP_Coord x1, GP_Coord y1)
{
	GP_TRANSFORM_POINT(backend->context, x0, y0);
	GP_TRANSFORM_POINT(backend->context, x1, y1);

	if (x1 < x0)
		GP_SWAP(x0, x1);
	
	if (y1 < y0)
		GP_SWAP(y0, y1);

	backend->UpdateRect(backend, x0, y0, x1, y1);
}
