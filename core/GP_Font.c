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
	GP_CHECK(font, "NULL font specified");

	return sizeof(GP_CharData) + font->bytes_per_line * font->height;
}

const GP_CharData *GP_GetCharData(const GP_Font *font, int c)
{
	GP_CHECK(font, "NULL font specified"); 

	/* characters before space are not encoded */
	switch (font->charset) {
	case GP_CHARSET_7BIT:
		if (c < 0x20 || c > 0x7f)
			return NULL;
	break;
	default:
		return NULL;
	}

	int encoded_character = c - ' ';

	/* NOTE: The character header is placed directly in the byte stream
	 * of character data without any alignment. If this makes problems
	 * on any machine, we will have to introduce appropriate padding.
	 */
	return (GP_CharData *)(font->data +
		GP_GetCharDataSize(font) * encoded_character);
}

int GP_GetCharCount(unsigned int charset)
{
	switch (charset) {
		case GP_CHARSET_7BIT:
			return 96;
	}

	GP_ABORT("Unrecognized font character set");
}

unsigned int GP_GetFontDataSize(const GP_Font *font)
{
	GP_CHECK(font, "NULL font specified");

	return GP_GetCharCount(font->charset) * GP_GetCharDataSize(font);
}

GP_RetCode GP_FontSave(const struct GP_Font *font, const char *filename)
{
	int retval;

	if (font == NULL || filename == NULL)
		return GP_ENULLPTR;

	FILE *f;

	f = fopen(filename, "w");
	if (f == NULL)
		return GP_EBADFILE;

	/* write file header */
	retval = fprintf(f,
		"%s\nFORMAT_VERSION %d.%d\nFAMILY %s\nNAME %s\nAUTHOR %s\n"
		"LICENSE %s\nVERSION %d\nGEOMETRY %d %d %d %d %d\n",
		GP_FONT_MAGIC,
		GP_FONT_FORMAT_VMAJOR, GP_FONT_FORMAT_VMINOR,
		font->family, font->name, font->author, font->license, font->version,
		font->charset, font->height, font->baseline,
		font->bytes_per_line, font->max_bounding_width);
	if (retval < 0)
		goto io_error;

	/* write character data */
	unsigned int char_data_size = GP_GetFontDataSize(font);
	retval = fwrite(font->data, char_data_size, 1, f);
	if (retval != 1)
		goto io_error;

	/* commit and close */
	if (fclose(f) != 0)
		return GP_EBADFILE;

	return GP_ESUCCESS;

io_error:
	fclose(f);
	return GP_EBADFILE;
}

GP_RetCode GP_FontLoad(GP_Font **pfont, const char *filename)
{
	int retval;
	int result = GP_EINVAL;

	if (pfont == NULL || filename == NULL)
		return GP_ENULLPTR;

	/* allocate the font metadata structure */
	GP_Font *font = (GP_Font *) calloc(1, sizeof(*font));
	if (font == NULL)
		return GP_ENOMEM;

	/* open the font file */
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		result = GP_ENOENT;
		goto bad;
	}

	/* check file magic and version */
	int format_vmajor, format_vminor;
	retval = fscanf(f, GP_FONT_MAGIC "\nFORMAT_VERSION %d.%d%*[\n]",
		&format_vmajor, &format_vminor);
	if (retval != 2) {
		fprintf(stderr, "gfxprim: error loading font: bad magic or version\n");
		result = GP_EBADFILE;
		goto bad;
	}

	/* read the header and fill the font metadata */
	retval = fscanf(f, "FAMILY %63[a-zA-Z0-9_ ]%*[\n]"
			"NAME %63[a-zA-Z0-9_ ]%*[\n]"
			"AUTHOR %63[a-zA-Z0-9_ ]%*[\n]",
			font->family, font->name, font->author);
	if (retval != 3) {
		fprintf(stderr, "gfxprim: error loading font: bad family, name, or author\n");
		result = GP_EBADFILE;
		goto bad;
	}

	retval = fscanf(f, "LICENSE %15s%*[\n]"
			"VERSION %u%*[\n]",
			font->license, &font->version);
	if (retval != 2) {
		if (retval == 0)
			fprintf(stderr, "gfxprim: error loading font: bad license string\n");
		else
			fprintf(stderr, "gfxprim: error loading font: bad version string\n");
		result = GP_EBADFILE;
		goto bad;
	}
	
	retval = fscanf(f, "GEOMETRY %hhu %hhu %hhu %hhu %hhu%*[\n]",
		&font->charset, &font->height,
		&font->baseline, &font->bytes_per_line,
		&font->max_bounding_width);
	if (retval != 5) {
		fprintf(stderr, "gfxprim: error loading font: bad geometry (item #%d)\n",
			retval);
		result = GP_EBADFILE;
		goto bad;
	}

	/* allocate memory for character data */
	unsigned int char_data_size = GP_GetFontDataSize(font);
	font->data = malloc(char_data_size);
	if (font->data == NULL) {
		fprintf(stderr, "gfxprim: error loading font: cannot allocate %u bytes\n",
			char_data_size);
		result = GP_ENOMEM;
		goto bad;
	}

	/* load character data */
	retval = fread(font->data, char_data_size, 1, f);
	if (retval != 1) {
		fprintf(stderr, "gfxprim: error loading font: premature end of character data\n");
		result = GP_EBADFILE;
		goto bad;
	}

	fclose(f);
	*pfont = font;
	return GP_ESUCCESS;

bad:
	if (f)
		fclose(f);
	if (font) {
		free(font->data);
		free(font);
	}
	return result;
}
