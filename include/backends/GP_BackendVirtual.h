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

#include <core/GP_Pixel.h>
#include <backends/GP_Backend.h>

enum gp_backend_virt_flags {
	/* If set virtual backend exit calls 'parent' exit as well */
	GP_BACKEND_CALL_EXIT = 0x01,
};

/*
 * Create an virtual backend on the top of the existing backend.
 */
gp_backend *gp_backend_virt_init(gp_backend *backend,
                                 gp_pixel_type pixel_type,
				 enum gp_backend_virt_flags flags);

#endif /* BACKENDS_GP_BACKEND_VIRTUAL_H */
