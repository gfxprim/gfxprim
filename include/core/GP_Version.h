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

    Library version.

  */

#ifndef CORE_GP_VERSION_H
#define CORE_GP_VERSION_H

#define GP_VER_MAJOR 0
#define GP_VER_MINOR 0
#define GP_VER_MICRO 0

#define GP_STRX(x) GP_STR(x)
#define GP_STR(x) #x

#define GP_VER_STR GP_STRX(GP_VER_MAJOR) "." \
                   GP_STRX(GP_VER_MINOR) "." \
                   GP_STRX(GP_VER_MICRO)

#endif /* CORE_GP_VERSION_H */
