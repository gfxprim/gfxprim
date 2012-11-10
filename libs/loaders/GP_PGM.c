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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  PGM portable graymap loader/saver.

  Format:
  
  a magick number value of 'P' and '2'
  whitespace (blanks, TABs, CRs, LFs).
  ascii width
  whitespace
  ascii height
  whitespace
  maximal gray value (interval is 0 ... max)
  width * height ascii gray values 
  
  lines starting with '#' are comments to the end of line
  
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <GP_Debug.h>

#include "GP_PXMCommon.h"
#include "GP_PGM.h"

static void try_read_comments(FILE *f)
{
	char c1, c2;

	while (isspace(c1 = fgetc(f)));
	
	ungetc(c1, f);

	while ((c1 = fgetc(f)) == '#') {
		do {
			c2 = fgetc(f);
		} while (c2 != '\n' && c2 != EOF);
	}

	ungetc(c1, f);
}

GP_Context *GP_LoadPGM(const char *src_path, GP_ProgressCallback *callback)
{
	FILE *f;
	GP_Context *ret;
	uint32_t w, h, gray;
	GP_PixelType type;
	char h1, h2;
	int err = EIO;
	
	f = fopen(src_path, "r");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open file '%s': %s",
		         src_path, strerror(errno));
		goto err0;
	}

	h1 = fgetc(f);
	h2 = fgetc(f);

	if (feof(f)) {
		err = EIO;
		goto err1;
	}

	if (h1 != 'P' || h2 != '2') {
		GP_DEBUG(1, "Invalid PGM header '%c%c' (0x%2x 0x%2x)",
		         isprint(h1) ? h1 : ' ', isprint(h2) ? h2 : ' ',
			 h1, h2);
		err = EINVAL;
		goto err1;
	}

	try_read_comments(f);

	if (fscanf(f, "%"PRIu32, &w) < 1) {
		err = errno;
		GP_DEBUG(1, "Failed to read PGM header width");
		goto err1;
	}
	
	try_read_comments(f);
	
	if (fscanf(f, "%"PRIu32, &h) < 1) {
		err = errno;
		GP_DEBUG(1, "Failed to read PGM header height");
		goto err1;
	}
	
	try_read_comments(f);
	
	if (fscanf(f, "%"PRIu32, &gray) < 1) {
		err = errno;
		GP_DEBUG(1, "Failed to read PGM header gray");
		goto err1;
	}
	
	switch (gray) {
	case 1:
		type = GP_PIXEL_G1;
	break;
	case 3:
		type = GP_PIXEL_G2;
	break;
	case 15:
		type = GP_PIXEL_G4;
	break;
	case 255:
		type = GP_PIXEL_G8;
	break;
	default:
		GP_DEBUG(1, "Invalid number of grays %u", gray);
		err = EINVAL;
		goto err1;
	}

	ret = GP_ContextAlloc(w, h, type);

	if (ret == NULL) {
		err = ENOMEM;
		goto err1;
	}

	//TODO: errno here
	switch (gray) {
	case 1:
		if (GP_PXMLoad1bpp(f, ret))
			goto err2;
	break;
	case 3:
		if (GP_PXMLoad2bpp(f, ret))
			goto err2;
	break;
	case 15:
		if (GP_PXMLoad4bpp(f, ret))
			goto err2;
	break;
	case 255:
		if (GP_PXMLoad8bpp(f, ret))
			goto err2;
	break;
	}

	fclose(f);
	return ret;
err2:
	GP_ContextFree(ret);
err1:
	fclose(f);
err0:
	errno = err;
	return NULL;
}

int GP_SavePGM(const GP_Context *src, const char *res_path,
               GP_ProgressCallback *callback)
{
	FILE *f;
	uint32_t gray;
	int err = EIO;

	switch (src->pixel_type) {
	case GP_PIXEL_G1:
		gray = 1;
	break;
	case GP_PIXEL_G2:
		gray = 3;
	break;
	case GP_PIXEL_G4:
		gray = 15;
	break;
	case GP_PIXEL_G8:
		gray = 255;
	break;
	default:
		GP_DEBUG(1, "Invalid pixel type '%s'",
		         GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1; 
	}
	
	f = fopen(res_path, "w");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open file '%s': %s",
		         res_path, strerror(errno));
		goto err0;
	}

	if (fprintf(f, "P2\n%u %u\n%u\n# Generated by gfxprim\n",
	            (unsigned int) src->w, (unsigned int) src->h, gray) < 3)
		goto err1;

	//TODO: errno
	switch (gray) {
	case 1:
		if (GP_PXMSave1bpp(f, src))
			goto err1;
	break;
	case 3:
		if (GP_PXMSave2bpp(f, src))
			goto err1;
	break;
	//TODO
	case 255:
		if (GP_PXMSave8bpp(f, src))
			goto err1;
	break;
	default:
		err = ENOSYS;
		goto err1;
	}

	if (fclose(f)) {
		err = errno;
		GP_DEBUG(1, "Failed to close file '%s': %s",
		         res_path, strerror(errno));
		goto err0;
	}

	return 0;
err1:
	fclose(f);
err0:
	errno = err;
	return 1;
}
