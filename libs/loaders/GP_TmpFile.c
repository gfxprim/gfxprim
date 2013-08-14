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

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "core/GP_Debug.h"

#include "GP_TmpFile.h"

static const char file_sig[] = {'G', 'F', 'X', 'p', 'r', 'i', 'm'};

GP_Context *GP_LoadTmpFile(const char *src_path, GP_ProgressCallback *callback)
{
	FILE *f;
	uint32_t w, h, y;
	uint8_t offset, flags;
	uint32_t bpr;
	enum GP_PixelType pixel_type;
	char sig[sizeof(file_sig)];
	GP_Context *ret;
	int err, i;

	f = fopen(src_path, "r");

	if (f == NULL)
		return NULL;

	/* Read an signature */
	if (fread(sig, sizeof(sig), 1, f) != 1) {
		err = EIO;
		goto err0;
	}

	if (strncmp(sig, file_sig, sizeof(sig))) {
		GP_WARN("Invalid file '%s' signature", src_path);
		err = EINVAL;
		goto err0;
	}

	/* Read context metadata */
	i  = fread(&w, sizeof(w), 1, f);
	i += fread(&h, sizeof(h), 1, f);
	i += fread(&offset, sizeof(offset), 1, f);
	i += fread(&bpr, sizeof(bpr), 1, f);
	i += fread(&pixel_type, sizeof(pixel_type), 1, f);
	i += fread(&flags, 1, 1, f);

	if (i != 6 || ferror(f)) {
		err = EIO;
		goto err0;
	}

	ret = GP_ContextAlloc(w, h, pixel_type);

	if (ret == NULL) {
		err = errno;
		goto err0;
	}

	//TODO: We may disagree here on ill aligned subcontexts
	GP_ASSERT(ret->bytes_per_row == bpr, "Invalid bytes per row");

	ret->offset = offset;

	/* And pixels */
	for (y = 0; y < h; y++) {
		if (fread(ret->pixels + bpr * y, bpr, 1, f) != 1) {
			err = EIO;
			goto err1;
		}

		GP_ProgressCallbackReport(callback, y, h, w);
	}

	/* Set the rotation flags */
	if (flags & 0x01)
		ret->axes_swap = 1;

	if (flags & 0x02)
		ret->x_swap = 1;

	if (flags & 0x04)
		ret->y_swap = 1;

	fclose(f);

	GP_ProgressCallbackDone(callback);

	return ret;
err1:
	GP_ContextFree(ret);
err0:
	fclose(f);
	errno = err;
	return NULL;
}

int GP_SaveTmpFile(const GP_Context *src, const char *dst_path,
                   GP_ProgressCallback *callback)
{
	FILE *f;
	int err;
	uint32_t y, i;

	f = fopen(dst_path, "w");

	if (f == NULL)
		return 1;

	/* Write a signature */
	i = fwrite(file_sig, sizeof(file_sig), 1, f);

	/* Write block of metadata */
	i += fwrite(&src->w, sizeof(src->w), 1, f);
	i += fwrite(&src->h, sizeof(src->h), 1, f);
	i += fwrite(&src->offset, sizeof(src->offset), 1, f);
	i += fwrite(&src->bytes_per_row, sizeof(src->bytes_per_row), 1, f);
	i += fwrite(&src->pixel_type, sizeof(src->pixel_type), 1, f);

	uint8_t flags = 0;

	if (src->axes_swap)
		flags |= 0x01;

	if (src->x_swap)
		flags |= 0x02;

	if (src->y_swap)
		flags |= 0x04;

	i += fwrite(&flags, 1, 1, f);

	if (i != 7) {
		err = EIO;
		goto err1;
	}

	/* And pixels */
	for (y = 0; y < src->h; y++) {
		if (fwrite(src->pixels + src->bytes_per_row * y, src->bytes_per_row, 1, f) != 1) {
			err = EIO;
			goto err1;
		}

		GP_ProgressCallbackReport(callback, y, src->h, src->w);
	}

	if (fclose(f)) {
		err = errno;
		goto err0;
	}

	GP_ProgressCallbackDone(callback);

	return 0;
err1:
	fclose(f);
err0:
	unlink(dst_path);
	errno = err;
	return 1;
}
