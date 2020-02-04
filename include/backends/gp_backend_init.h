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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Simplified backend initalization interface good enough for most of the cases.

 */

#ifndef BACKENDS_GP_BACKEND_INIT_H
#define BACKENDS_GP_BACKEND_INIT_H

#include <backends/gp_backend.h>

/*
 * Initalize backend by a string.
 *
 * The format is:
 *
 * "backend_name:backend_params"
 *
 * For example "SDL:fs" is string for fullscreen SDL backend.
 *
 * The caption parameter may, or may not be used. For example in windowed
 * enviroment caption will become caption of a window. When running on
 * framebuffer it may be ignored completly.
 *
 * Returns initalized backend or NULL in case of failure.
 *
 * If help is passed as a backend name a help is printed into the stderr.
 */
gp_backend *gp_backend_init(const char *params, const char *caption);

#endif /* BACKENDS_GP_BACKEND_INIT_H */
