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

#ifndef GP_CLIP_H
#define GP_CLIP_H

/*
 * Clipping rectanle mirroring and rotations.
 */
#define GP_MIRROR_V_CLIP(context) do {                                 \
	typeof(context->clip_w_min) _clip_w_min = context->clip_w_min; \
	                                                               \
	context->clip_w_min = context->w - context->clip_w_max;        \
	context->clip_w_max = context->w - _clip_w_min;                \
} while (0)

#define GP_MIRROR_H_CLIP(context) do {                                 \
	typeof(context->clip_h_min) _clip_h_min = context->clip_h_min; \
	                                                               \
	context->clip_h_min = context->h - context->clip_h_max;        \
	context->clip_h_max = context->h - _clip_h_min;                \
} while (0)

#define GP_SWAP_CLIPS(context) do {                        \
	GP_SWAP(context->clip_w_min, context->clip_h_min); \
	GP_SWAP(context->clip_w_max, context->clip_h_max); \
} while (0)

#endif /* GP_CLIP_H */
