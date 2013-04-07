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

   Utils to read/write bytes in specified endianity.

  */

#ifndef LOADERS_GP_BYTE_UTILS_H
#define LOADERS_GP_BYTE_UTILS_H

#include <stdio.h>

/*
 * The format string examples:
 *
 * %l1 or %b1 assigns one byte
 * %l2 assign two bytes in little endian
 * %b4 assign four bytes in big endian
 * %a6 read six bytes into array
 *
 * To read and write header with two byte signature, two reserved zero bytes
 * and size in 16 bit unsigned little endian variables.
 *
 * uint16_t w;
 * uint16_t h;
 * char sig[2];
 *
 * if (GP_FWrite(f, "%a2%x00%x00%l2%l2", "SG", w, h) != 5)
 * 	//ERROR
 *
 * if (GP_FRead(f, "%a2%x00%x00%l2%l2", sig, &w, &h) != 5)
 * 	//ERROR
 */

/*
 * Printf-like function to read bytes from a file.
 *
 * Returns number of characters read.
 */
int GP_FRead(FILE *f, const char *fmt, ...);

/*
 * Printf-like function to write bytes to a file.
 *
 * Returns number of characters read.
 */
int GP_FWrite(FILE *f, const char *fmt, ...);

#endif /* LOADERS_GP_BYTE_UTILS_H */
