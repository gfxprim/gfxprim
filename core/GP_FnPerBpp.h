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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP.h"

/*
 * Macro that generates a switch-case block that calls various variants
 * of the specified function depending on the bit depth of the context.
 * Extra arguments are arguments to be passed to the function.
 * Returns GP_ENOIMPL if the bit depth is unknown.
 *
 * Note: Relying on existing context variable is ugly and broken, I know...
 *       But I hate doing just another GP_FN_PER_BPP macro for functions
 *       that takes context as it's only argument. Or passing the context
 *       twice or whatever else.
 */
#define GP_FN_PER_BPP(FN_NAME, ...) \
\
	switch (context->bpp) { \
	case 1: \
		FN_NAME##1bpp(__VA_ARGS__); \
	break; \
	case 2: \
		FN_NAME##2bpp(__VA_ARGS__); \
	break; \
	case 4: \
		FN_NAME##4bpp(__VA_ARGS__); \
	break; \
	case 8: \
		FN_NAME##8bpp(__VA_ARGS__); \
	break; \
	case 16: \
		FN_NAME##16bpp(__VA_ARGS__); \
	break; \
	case 24: \
		FN_NAME##24bpp(__VA_ARGS__); \
	break; \
	case 32: \
		FN_NAME##32bpp(__VA_ARGS__); \
	break; \
	default: \
		return GP_ENOIMPL; \
	} \
\
	return GP_ESUCCESS;

#define GP_FN_RET_PER_BPP(FN_NAME, ...) \
\
	switch (context->bpp) { \
	case 1: \
		return FN_NAME##1bpp(__VA_ARGS__); \
	case 2: \
		return FN_NAME##2bpp(__VA_ARGS__); \
	case 4: \
		return FN_NAME##4bpp(__VA_ARGS__); \
	case 8: \
		return FN_NAME##8bpp(__VA_ARGS__); \
	case 16: \
		return FN_NAME##16bpp(__VA_ARGS__); \
	case 24: \
		return FN_NAME##24bpp(__VA_ARGS__); \
	case 32: \
		return FN_NAME##32bpp(__VA_ARGS__); \
	}
