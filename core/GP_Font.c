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

#include "GP.h"

#include <stdio.h>

unsigned int GP_GetCharDataSize(const GP_Font *font)
{
	GP_CHECK(font != NULL);

	return sizeof(GP_CharData) + font->bytes_per_line * font->height;
}

const GP_CharData *GP_GetCharData(const GP_Font *font, int c)
{
	GP_CHECK(font != NULL && c >= ' ');

	/* Characters before space are not encoded. */
	int encoded_character = c - ' ';

	/* NOTE: The character header is placed directly in the byte stream
	 * of character data without any alignment. If this makes problems
	 * on any machine, we will have to introduce appropriate padding.
	 */
	return (GP_CharData *)(font->data +
		GP_GetCharDataSize(font) * encoded_character);
}

GP_RetCode GP_FontSave(const struct GP_Font *font, const char *filename)
{
	if (font == NULL || filename == NULL)
		return GP_ENULLPTR;

	/* calculate the number of characters */
	unsigned int char_count;
	switch (font->charset) {
		case GP_CHARSET_7BIT:
			char_count = 96;
			break;

		default:
			return GP_EINVAL;
	}

	FILE *f;

	f = fopen(filename, "w");
	if (f == NULL)
		return GP_EBADFILE;

	/* font file signature and version */
	fputs("# gfxprim font\n", f);
	fprintf(f, "%d.%d\n", GP_FONT_FORMAT_VMAJOR,
		GP_FONT_FORMAT_VMINOR);

	/* font header */
	fprintf(f, "%s\n", font->family);
	fprintf(f, "%s\n", font->name);
	fprintf(f, "%s\n", font->license);
	fprintf(f, "%d\n", font->version);
	fprintf(f, "%d %d %d %d\n", font->charset, font->height,
		font->baseline, font->bytes_per_line, font->max_bounding_width);

	/* check if no I/O errors occurred so far */
	if (ferror(f))
		goto io_error;

	/* write character data */
	unsigned int char_data_size = char_count * GP_GetCharDataSize(font);
	if (fwrite(font->data, char_data_size, 1, f) != 1)
		goto io_error;

	if (fclose(f) != 0)
		return GP_EBADFILE;

	return GP_ESUCCESS;

io_error:
	fclose(f);
	return GP_EBADFILE;
}

