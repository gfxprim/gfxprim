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

#include "core/GP_Core.h"
#include "core/GP_FnPerBpp.h"
#include "core/GP_DefFnPerBpp.h"

#include "GP_Rotate.h"

#include <string.h>

#include "algo/GP_MirrorV.algo.h"

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

	return GP_ESUCCESS;
}

/* Generate mirror functions per BPP */
GP_DEF_FFN_PER_BPP(GP_MirrorV, DEF_MIRRORV_FN)

GP_RetCode GP_MirrorV(GP_Context *context)
{
	if (context == NULL)
		return GP_ENULLPTR;

	GP_FN_PER_BPP_CONTEXT(GP_MirrorV, context, context);

	return GP_ESUCCESS;
}

#include "algo/GP_Rotate.algo.h"

/* Generate Rotate functions per BPP */
GP_DEF_FFN_PER_BPP(GP_RotateCW, DEF_ROTATECW_FN)

GP_RetCode GP_RotateCW(GP_Context *context)
{
	if (context == NULL)
		return GP_ENULLPTR;

	GP_FN_RET_PER_BPP_CONTEXT(GP_RotateCW, context, context);

	return GP_ENOIMPL;
}

/* Generate Rotate functions per BPP */
GP_DEF_FFN_PER_BPP(GP_RotateCCW, DEF_ROTATECCW_FN)

GP_RetCode GP_RotateCCW(GP_Context *context)
{
	if (context == NULL)
		return GP_ENULLPTR;

	GP_FN_RET_PER_BPP_CONTEXT(GP_RotateCCW, context, context);

	return GP_ENOIMPL;
}
