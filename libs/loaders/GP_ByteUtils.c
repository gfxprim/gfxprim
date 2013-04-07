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

#include <stdarg.h>

#include "core/GP_Debug.h"

#include "loaders/GP_ByteUtils.h"

enum fmt_type {
	BYTE_ARRAY,
	BIG_ENDIAN_VAR,
	LITTLE_ENDIAN_VAR,
	IGNORE,
	CONST_BYTE,
};

static int ctoi(char c, int *i)
{
	switch (c) {
	case '0' ... '9':
		*i += c - '0';
	break;
	default:
		return 1;
	}

	return 0;
}

static int ctoh(char c, int *i)
{
	switch (c) {
	case '0' ... '9':
		*i += c - '0';
	break;
	case 'a' ... 'f':
		*i += c - 'a' + 10;
	break;
	case 'A' ... 'F':
		*i += c - 'A' + 10;
	break;
	case '\0':
		GP_WARN("Unexpected end of the format string");
		return 1;
	default:
		GP_WARN("Expected [0-9]|[a-f][A-F] in hex constant, got '%c'", c);
		return 1;
	}

	return 0;
}

static const char *get_hex(const char *fmt, int *type, int *val)
{
	*type = CONST_BYTE;

	*val = 0;

	if (ctoh(fmt[0], val))
		return NULL;

	(*val)<<=4;

	if (ctoh(fmt[1], val))
		return NULL;

	return fmt + 2;
}

static const char *get_int(const char *fmt, int *val)
{
	int i = 0;

	*val = 0;

	if (ctoi(fmt[i++], val))
		return fmt;
	
	while (!ctoi(fmt[i], val)) {
		*val *= 10;
		i++;
	} 

	return fmt + i;
}

static const char *get_array(const char *fmt, int *type, int *val)
{
	*type = BYTE_ARRAY;

	fmt = get_int(fmt, val);

	/* array for one element "%a" */
	if (*val == 0)
		*val = 1;

	return fmt;
}

static const char *get_lb_size(const char *fmt, int *val)
{
	*val = 0;

	if (ctoi(fmt[0], val)) {
		GP_WARN("Expected number after %%l or %%b, got '%c'", fmt[0]);
		return NULL;
	}

	switch (*val) {
	case 1:
	case 2:
	case 4:
		return fmt + 1;
	}

	GP_WARN("Invalid little/big endian variable size '%i'", *val);
	return NULL;
}

static const char *get_next(const char *fmt, int *type, int *val)
{
	if (fmt[0] == '\0')
		return NULL;

	if (fmt[0] != '%') {
		GP_WARN("Unexpected character in format string '%c'", fmt[0]);
		return NULL;
	}

	switch (fmt[1]) {
	/* byte array */
	case 'a':
		return get_array(fmt + 2, type, val);
	break;
	/* hexadecimal constant */
	case 'x':
		return get_hex(fmt + 2, type, val);
	break;
	/* 1, 2, 4 bytes long variable in defined endianity */
	case 'l':
		*type = LITTLE_ENDIAN_VAR;
		return get_lb_size(fmt + 2, val);
	break;
	case 'b':
		*type = BIG_ENDIAN_VAR;
		return get_lb_size(fmt + 2, val);
	break;
	case 'i':

	break;
	case '\0':
		GP_WARN("Unexpecned end of format string");
		return NULL;
	break;
	}

	GP_WARN("Unexpected character in format string '%c'", fmt[0]);
	return NULL;
}

int GP_FRead(FILE *f, const char *fmt, ...)
{
	int type, val, ret = 0;
	va_list va;

	va_start(va, fmt);

	for (;;) {
		fmt = get_next(fmt, &type, &val);

		/* end of the string or error */
		if (fmt == NULL)
			goto end;

		switch (type) {
		case BYTE_ARRAY:
			if (fread(va_arg(va, void*), val, 1, f) != 1)
				goto end;
		break;
		case CONST_BYTE:
			if (fgetc(f) != val)
				goto end;
		break;
		case LITTLE_ENDIAN_VAR:
		case BIG_ENDIAN_VAR:
			if (fread(va_arg(va, void*), val, 1, f) != 1)
				goto end;
			//TODO: Fix byteorder
		break;
		case IGNORE:
			
		break;
		}
		
		ret++;

	}
end:
	va_end(va);
	return ret;
}

int GP_FWrite(FILE *f, const char *fmt, ...)
{
	int type, val, ret = 0;
	va_list va;

	va_start(va, fmt);

	for (;;) {
		fmt = get_next(fmt, &type, &val);

		/* end of the string or error */
		if (fmt == NULL)
			goto end;

		switch (type) {
		case BYTE_ARRAY:
			if (fwrite(va_arg(va, void*), val, 1, f) != 1)
				goto end;
		break;
		case CONST_BYTE:
			if (fwrite(&val, 1, 1, f) != 1)
				goto end;
		break;
		case LITTLE_ENDIAN_VAR:
		case BIG_ENDIAN_VAR:
			switch (val) {
			case 1: {
				uint8_t c = va_arg(va, int);
				if (fwrite(&c, 1, 1, f) != 1)
					goto end;
			} break;
			case 2: {
				uint16_t c = va_arg(va, int);
				if (fwrite(&c, 2, 1, f) != 1)
					goto end;
			} break;
			case 4: {
				uint32_t c = va_arg(va, int);
				if (fwrite(&c, 4, 1, f) != 1)
					goto end;
			} break;
			}
		break;
		default:
			GP_WARN("Wrong format type for writing");
			goto end;
		}

		ret++;
	}
end:
	va_end(va);
	return ret;
}
