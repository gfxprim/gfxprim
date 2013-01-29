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

#ifndef __BOGOMAN_DEBUG_H__
#define __BOGOMAN_DEBUG_H__

#define DEBUG(level, ...) \
        bogoman_dbg_print(level, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define WARN(...) \
        bogoman_dbg_print(0, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)


void bogoman_dbg_print(unsigned int level, const char *file, const char *fn,
                       unsigned int line, const char *fmt, ...)
                       __attribute__ ((format (printf, 5, 6)));

void bogoman_set_dbg_level(unsigned int level);

#endif /* __BOGOMAN_DEBUG_H__ */
