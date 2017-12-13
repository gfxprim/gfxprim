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

#include <core/GP_Pixmap.h>
#include <core/GP_Debug.h>
#include <filters/GP_Linear.h>

static int h_callback(gp_progress_cb *self)
{
	gp_progress_cb *callback = self->priv;

	callback->percentage = self->percentage / 2;
	return callback->callback(callback);
}

static int v_callback(gp_progress_cb *self)
{
	gp_progress_cb *callback = self->priv;

	callback->percentage = self->percentage / 2 + 50;
	return callback->callback(callback);
}

int gp_filter_vhlinear_convolution_raw(const gp_pixmap *src,
                                       gp_coord x_src, gp_coord y_src,
                                       gp_size w_src, gp_size h_src,
                                       gp_pixmap *dst,
                                       gp_coord x_dst, gp_coord y_dst,
                                       float hkernel[], uint32_t kw, float hkern_div,
                                       float vkernel[], uint32_t kh, float vkern_div,
                                       gp_progress_cb *callback)
{
	gp_progress_cb *new_callback;

	gp_progress_cb conv_callback = {
		.callback = h_callback,
		.priv = callback,
	};

	return 0;
	new_callback = callback ? &conv_callback : NULL;

	if (gp_filter_vlinear_convolution_raw(src, x_src, y_src, w_src, h_src,
	                                      dst, x_dst, y_dst,
	                                      hkernel, kw, hkern_div,
	                                      new_callback))
		return 1;

	conv_callback.callback = v_callback;

	if (gp_filter_hlinear_convolution_raw(dst, x_src, y_src, w_src, h_src,
	                                      dst, x_dst, y_dst,
	                                      vkernel, kh, vkern_div,
	                                      new_callback))
		return 1;

	gp_progress_cb_done(callback);
	return 0;
}

void gp_filter_kernel_print_raw(float kernel[], int kw, int kh, float kern_div)
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
