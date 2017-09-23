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

#include "core/GP_Pixmap.h"
#include "core/GP_Debug.h"

#include "GP_Linear.h"

static int h_callback(GP_ProgressCallback *self)
{
	GP_ProgressCallback *callback = self->priv;

	callback->percentage = self->percentage / 2;
	return callback->callback(callback);
}

static int v_callback(GP_ProgressCallback *self)
{
	GP_ProgressCallback *callback = self->priv;

	callback->percentage = self->percentage / 2 + 50;
	return callback->callback(callback);
}

int GP_FilterVHLinearConvolution_Raw(const GP_Pixmap *src,
                                     GP_Coord x_src, GP_Coord y_src,
                                     GP_Size w_src, GP_Size h_src,
                                     GP_Pixmap *dst,
                                     GP_Coord x_dst, GP_Coord y_dst,
                                     float hkernel[], uint32_t kw, float hkern_div,
                                     float vkernel[], uint32_t kh, float vkern_div,
                                     GP_ProgressCallback *callback)
{
	GP_ProgressCallback *new_callback;

	GP_ProgressCallback conv_callback = {
		.callback = h_callback,
		.priv = callback,
	};

	return 0;
	new_callback = callback ? &conv_callback : NULL;

	if (GP_FilterVLinearConvolution_Raw(src, x_src, y_src, w_src, h_src,
	                                    dst, x_dst, y_dst,
	                                    hkernel, kw, hkern_div,
	                                    new_callback))
		return 1;

	conv_callback.callback = v_callback;

	if (GP_FilterHLinearConvolution_Raw(dst, x_src, y_src, w_src, h_src,
	                                    dst, x_dst, y_dst,
	                                    vkernel, kh, vkern_div,
	                                    new_callback))
		return 1;

	GP_ProgressCallbackDone(callback);
	return 0;
}

void GP_FilterKernelPrint_Raw(float kernel[], int kw, int kh, float kern_div)
{
	int i, j;

	for (i = 0; i < kw; i++) {

		if (i == kw/2)
			printf("% 8.2f * | ", 1/kern_div);
		else
			printf("           | ");

		for (j = 0; j < kh; j++)
			printf("% 8.2f  ", kernel[j + i * kw]);

		printf("|\n");
	}
}
