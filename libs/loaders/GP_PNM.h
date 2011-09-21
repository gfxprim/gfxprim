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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

 Common PNM functions.

 */

#ifndef GP_PNM_H
#define GP_PNM_H

#include <stdint.h>

/*
 * Loads image header, returns pointer to FILE* (with file possition pointing
 * to the start of the data stream) on success, fills image metadata into
 * arguments.
 */
FILE *GP_ReadPNM(const char *src_path, char *fmt,
                 uint32_t *w, uint32_t *h, uint32_t *depth);

/*
 * Writes image header.
 */
FILE *GP_WritePNM(const char *dst_path, char fmt,
                  uint32_t w, uint32_t h, uint32_t depth);
                        

#endif /* GP_PNM_H */
