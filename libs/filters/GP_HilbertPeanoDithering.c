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

#include "core/GP_Debug.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Core.h"

#include "GP_HilbertCurve.h"
#include "GP_Dither.h"

/*
 * Returns closest greater square of two, used to determine the curve size.
 */
static unsigned int count_bits(unsigned int n)
{
	unsigned int i = 0, s = n;

	do {
		n>>=1;
		i++;
	} while (n);

	return (i + (s != (1<<i)));
}

int GP_FilterHilbertPeano_from_RGB888(const GP_Context *src,
                                      GP_Context *dst,
                                      GP_ProgressCallback *callback)
{
	struct GP_CurveState state;

	unsigned int n = GP_MAX(count_bits(src->w), count_bits(src->h));

	GP_DEBUG(1, "Hilbert Peano dithering %ux%u -> n = %u", src->w, src->h, n);

	GP_HilbertCurveInit(&state, n);
	
	int err = 0, cnt = 0;

	while (GP_HilbertCurveContinues(&state)) {
		
		if (state.x < src->w && state.y < src->h) {
			int pix = GP_GetPixel_Raw(src, state.x, state.y);
			
			pix = GP_ConvertPixel(pix, src->pixel_type, GP_PIXEL_G8);

			pix += err;

			if (pix > 127) {
				err = pix - 255;
				pix = 1;
			} else {
				err = pix;
				pix = 0;
			}

			GP_PutPixel_Raw_1BPP_LE(dst, state.x, state.y, pix);

			cnt++;

			if (GP_ProgressCallbackReport(callback, cnt/src->h, src->w, src->h))
				return 1;

			/* We are done, exit */
			if (cnt == src->w * src->h - 1) {
				GP_ProgressCallbackDone(callback);
				return 0;
			}
		} else {
			err = 0;
		}

		GP_HilbertCurveNext(&state);
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}
