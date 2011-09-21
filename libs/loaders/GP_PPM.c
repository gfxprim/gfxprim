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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  PPM portable bitmap loader/saver.
  
 */

#include <stdint.h>

#include <GP_Debug.h>
#include <GP_Context.h>
#include <GP_Pixel.h>
#include <GP_GetPutPixel.h>

#include "GP_PXMCommon.h"

int load_binary_ppm(FILE *f, uint32_t w, uint32_t h, uint32_t depth,
                    GP_Context *res)
{
	uint32_t x, y;
	int r, g, b;

	for (x = 0; x < w; x++)
		for (y = 0; y < h; y++) {
			r = fgetc(f);
			g = fgetc(f);
			b = fgetc(f);
		
			if (r == EOF || g == EOF || b == EOF) {
				GP_DEBUG(1, "Unexpected end of PBM file");
				return 1;
			}
			
			//TODO depth
			GP_Pixel pix = GP_Pixel_CREATE_RGB888(r, g, b);
			GP_PutPixel_Raw_24BPP(res, x, y, pix);
		}

	return 0;
}

GP_RetCode GP_LoadPPM(const char *src_path, GP_Context **res)
{
	uint32_t w, h, depth;
	char fmt;
	FILE *f;

	f = GP_OpenPNM(src_path, &fmt, &w, &h, &depth);

	if (f == NULL)
		return GP_EBADFILE;

	if (fmt != '3' && fmt != '6') {
		GP_DEBUG(1, "Asked to load PPM but header is 'P%c'", fmt);
		goto err1;
	}

	*res = GP_ContextAlloc(w, h, GP_PIXEL_RGB888);

	if (res == NULL)
		goto err1;

	switch (fmt) {
	case '3':
		//TODO
		fclose(f);
		free(res);
		return GP_ENOIMPL;
	case '6':
		if (load_binary_ppm(f, w, h, depth, *res))
			goto err2;
	break;
	}

	fclose(f);
	return GP_ESUCCESS;
err2:
	free(*res);
err1:
	fclose(f);
	return GP_EBADFILE;
}

GP_RetCode GP_SavePPM(const char *res_path, GP_Context *src)
{
	return GP_ENOIMPL;
}
