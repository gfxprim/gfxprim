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
#include "GP_FnPerBpp.h"

#include "GP_MirrorV.algo.h"
#include "GP_Rotate.algo.h"

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

	GP_MIRROR_H_CLIP(context);

	return GP_ESUCCESS;
}

DEF_MIRRORV_FN(GP_MirrorV1bpp, GP_Context *, GP_Pixel,
               GP_PUTPIXEL_1BPP, GP_GETPIXEL_1BPP)
DEF_MIRRORV_FN(GP_MirrorV2bpp, GP_Context *, GP_Pixel,
               GP_PUTPIXEL_2BPP, GP_GETPIXEL_2BPP)
DEF_MIRRORV_FN(GP_MirrorV4bpp, GP_Context *, GP_Pixel,
               GP_PUTPIXEL_4BPP, GP_GETPIXEL_4BPP)
DEF_MIRRORV_FN(GP_MirrorV8bpp, GP_Context *, GP_Pixel,
               GP_PUTPIXEL_8BPP, GP_GETPIXEL_8BPP)
DEF_MIRRORV_FN(GP_MirrorV16bpp, GP_Context *, GP_Pixel,
               GP_PUTPIXEL_16BPP, GP_GETPIXEL_16BPP)
DEF_MIRRORV_FN(GP_MirrorV24bpp, GP_Context *, GP_Pixel,
               GP_PUTPIXEL_24BPP, GP_GETPIXEL_24BPP)
DEF_MIRRORV_FN(GP_MirrorV32bpp, GP_Context *, GP_Pixel,
               GP_PUTPIXEL_32BPP, GP_GETPIXEL_32BPP)

GP_RetCode GP_MirrorV(GP_Context *context)
{
	if (context == NULL)
		return GP_ENULLPTR;

	GP_FN_PER_BPP(GP_MirrorV, context);
}

DEF_ROTATECW_FN(GP_RotateCW1bpp, GP_Context *, GP_PUTPIXEL_1BPP, GP_GETPIXEL_1BPP)
DEF_ROTATECW_FN(GP_RotateCW2bpp, GP_Context *, GP_PUTPIXEL_2BPP, GP_GETPIXEL_2BPP)
DEF_ROTATECW_FN(GP_RotateCW4bpp, GP_Context *, GP_PUTPIXEL_4BPP, GP_GETPIXEL_4BPP)
DEF_ROTATECW_FN(GP_RotateCW8bpp, GP_Context *, GP_PUTPIXEL_8BPP, GP_GETPIXEL_8BPP)
DEF_ROTATECW_FN(GP_RotateCW16bpp, GP_Context *, GP_PUTPIXEL_16BPP, GP_GETPIXEL_16BPP)
DEF_ROTATECW_FN(GP_RotateCW24bpp, GP_Context *, GP_PUTPIXEL_24BPP, GP_GETPIXEL_24BPP)
DEF_ROTATECW_FN(GP_RotateCW32bpp, GP_Context *, GP_PUTPIXEL_32BPP, GP_GETPIXEL_32BPP)

GP_RetCode GP_RotateCW(GP_Context *context)
{
	if (context == NULL)
		return GP_ENULLPTR;

	GP_FN_RET_PER_BPP(GP_RotateCW, context);

	return GP_ENOIMPL;
}

DEF_ROTATECCW_FN(GP_RotateCCW1bpp, GP_Context *, GP_PUTPIXEL_1BPP, GP_GETPIXEL_1BPP)
DEF_ROTATECCW_FN(GP_RotateCCW2bpp, GP_Context *, GP_PUTPIXEL_2BPP, GP_GETPIXEL_2BPP)
DEF_ROTATECCW_FN(GP_RotateCCW4bpp, GP_Context *, GP_PUTPIXEL_4BPP, GP_GETPIXEL_4BPP)
DEF_ROTATECCW_FN(GP_RotateCCW8bpp, GP_Context *, GP_PUTPIXEL_8BPP, GP_GETPIXEL_8BPP)
DEF_ROTATECCW_FN(GP_RotateCCW16bpp, GP_Context *, GP_PUTPIXEL_16BPP, GP_GETPIXEL_16BPP)
DEF_ROTATECCW_FN(GP_RotateCCW24bpp, GP_Context *, GP_PUTPIXEL_24BPP, GP_GETPIXEL_24BPP)
DEF_ROTATECCW_FN(GP_RotateCCW32bpp, GP_Context *, GP_PUTPIXEL_32BPP, GP_GETPIXEL_32BPP)

GP_RetCode GP_RotateCCW(GP_Context *context)
{
	if (context == NULL)
		return GP_ENULLPTR;

	GP_FN_RET_PER_BPP(GP_RotateCCW, context);

	return GP_ENOIMPL;
}
