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

  Virtual backend, could emulate backed with any pixel type on the top of
  initalized backend. Useful for testing.

 */

#ifndef BACKENDS_GP_BACKEND_VIRTUAL_H
#define BACKENDS_GP_BACKEND_VIRTUAL_H

#include "GP_Backend.h"

enum GP_BackendVirtFlags {
	/*
	 * If set virtual backend exit calls 'parent' Exit as well.
	 */
	GP_BACKEND_CALL_EXIT = 0x01,
};

/*
 * Create an virtual backend on the top of the existing backend.
 */
GP_Backend *GP_BackendVirtualInit(GP_Backend *backend,
                                  GP_PixelType pixel_type, int flags);

#endif /* BACKENDS_GP_BACKEND_VIRTUAL_H */
