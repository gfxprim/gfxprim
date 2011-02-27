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

#include "GP_Rotate.h"

#include "GP_GetPixel.h"
#include "GP_PutPixel.h"

#include "GP_Swap.h"

#include <string.h>

GP_RetCode GP_MirrorH(GP_Context *context)
{
	uint32_t bpr = context->bytes_per_row;
	uint8_t  buf[bpr];
	uint32_t y;

	if (context == NULL)
		return GP_ENULLPTR;

	for (y = 0; y < context->h/2; y++) {
		uint8_t *l1 = context->pixels + bpr * y;
		uint8_t *l2 = context->pixels + bpr * (context->h - y - 1);

		memcpy(buf, l1, bpr);
		memcpy(l1, l2, bpr);
		memcpy(l2, buf, bpr);
	}

	//TODO clipping

	return GP_ESUCCESS;
}

//TODO: Use per bpp functions?
GP_RetCode GP_MirrorV(GP_Context *context)
{
	uint32_t x, y;
	GP_Pixel tmp;

	if (context == NULL)
		return GP_ENULLPTR;

	for (x = 0; x < context->w/2; x++) {
		uint8_t xm = context->w - x - 1;
		for (y = 0; y < context->h; y++) {
			tmp = GP_GetPixel(context, x, y);
			
			GP_PutPixel(context, x, y, GP_GetPixel(context, xm, y));
			GP_PutPixel(context, xm, y, tmp);
		}
	}

	//TODO: swap clipping

	return GP_ESUCCESS;
}

//TODO: Use per bpp functions?
//TODO: in place rotation?
static GP_RetCode rotate_c(struct GP_Context *context, int flag)
{
	uint32_t x, y;
	GP_Context *tmp;

	if (context == NULL)
		return GP_ENULLPTR;

	tmp = GP_ContextCopy(context, GP_COPY_WITH_PIXELS);

	if (tmp == NULL)
		return GP_ENOMEM;

	/* rotate context metadata */
	GP_SWAP(context->w, context->h);

	GP_SWAP(context->clip_w_min, context->clip_h_min);
	GP_SWAP(context->clip_w_max, context->clip_h_max);

	context->bytes_per_row = GP_CALC_ROW_SIZE(context->pixel_type,
	                                          context->w);
	
	for (x = 0; x < tmp->w; x++) {
		for (y = 0; y < tmp->h; y++) {
			if (flag)
				GP_PutPixel(context, tmp->h - y - 1, x,
				            GP_GetPixel(tmp, x, y));
			else
				GP_PutPixel(context, y, tmp->w - x - 1,
				            GP_GetPixel(tmp, x, y));
		}
	}

	GP_ContextFree(tmp);

	return GP_ESUCCESS;
}


GP_RetCode GP_RotateCW(GP_Context *context)
{
	return rotate_c(context, 1);
}

GP_RetCode GP_RotateCCW(GP_Context *context)
{
	return rotate_c(context, 0);
}
