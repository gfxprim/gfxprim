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

  Simplified backend initalization interface good enough for most of the cases.

 */

#ifndef BACKENDS_GP_BACKEND_INIT_H
#define BACKENDS_GP_BACKEND_INIT_H

#include "backends/GP_Backend.h"

/*
 * Initalize backend by a string.
 *
 * The format is:
 *
 * "backend_name:backend_params"
 *
 * For example "SDL:FS" is string for fullscreen SDL backend.
 *
 * The caption parameter may, or may not be used. For example in windowed
 * enviroment caption will become caption of a window. When running on
 * framebuffer it may be ignored completly.
 *
 * Returns initalized backend or NULL in case of failure.
 *
 * If initialization has failed or params is NULL and help is not NULL, help
 * text is printed to a given file.
 */
GP_Backend *GP_BackendInit(const char *params, const char *caption, FILE *help);

#endif /* BACKENDS_GP_BACKEND_INIT_H */
