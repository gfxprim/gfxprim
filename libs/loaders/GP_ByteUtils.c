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

#ifdef __linux__
#include <endian.h>
#else /* BSD Family */
#include <machine/endian.h>
#endif

#include "core/GP_Debug.h"
#include "core/GP_Common.h"

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
		*i = c - '0';
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
		GP_BUG("Unexpected end of the format string");
		return 1;
	default:
		GP_BUG("Expected [0-9]|[a-f][A-F] in hex constant, got '%c'", c);
		return 1;
	}

	return 0;
}

static const char *get_hex(const char *fmt, int *type, int *val)
{
	*type = CONST_BYTE;

	*val = 0;

	if (fmt[0] != 'x') {
		GP_BUG("Expected x after 0 in hex constant");
		return NULL;
	}

	if (ctoh(fmt[1], val))
		return NULL;

	(*val)<<=4;

	if (ctoh(fmt[2], val))
		return NULL;

	return fmt + 3;
}

static const char *get_int(const char *fmt, int *val)
{
	int i = 0, add = 0;

	*val = 0;

	if (ctoi(fmt[i++], val))
		return fmt;

	while (!ctoi(fmt[i], &add)) {
		*val *= 10;
		*val += add;
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
	if (ctoi(fmt[0], val)) {
		GP_WARN("Expected number got '%c'", fmt[0]);
		return NULL;
	}

	switch (*val) {
	case 1:
	case 2:
	case 4:
		return fmt + 1;
	}

	GP_BUG("Invalid little/big endian variable size '%i'", *val);
	return NULL;
}

static const char *get_next(const char *fmt, int *type, int *val)
{
	/* Eat spaces */
	while (fmt[0] == ' ')
		fmt++;

	switch (fmt[0]) {
	/* Byte array */
	case 'A':
		return get_array(fmt + 1, type, val);
	break;
	/* Hexadecimal constant */
	case '0':
		return get_hex(fmt + 1, type, val);
	break;
	/* 1, 2, 4 bytes long variable in defined endianity */
	case 'L':
		*type = LITTLE_ENDIAN_VAR;
		return get_lb_size(fmt + 1, val);
	break;
	case 'B':
		*type = BIG_ENDIAN_VAR;
		return get_lb_size(fmt + 1, val);
	break;
	case 'I':
		*type = IGNORE;
		return get_int(fmt + 1, val);
	break;
	case '\0':
		return NULL;
	break;
	}

	GP_BUG("Unexpected character in format string '%c'", fmt[0]);
	return NULL;
}

static void swap_bytes(void *ptr, int len, int type)
{
	if (__BYTE_ORDER == __LITTLE_ENDIAN && type == LITTLE_ENDIAN_VAR)
		return;

	if (__BYTE_ORDER == __BIG_ENDIAN && type == BIG_ENDIAN_VAR)
		return;

	char *buf = ptr;

	switch (len) {
	case 1:
	break;
	case 2:
		GP_SWAP(buf[0], buf[1]);
	break;
	case 4:
		GP_SWAP(buf[0], buf[3]);
		GP_SWAP(buf[1], buf[2]);
	break;
	default:
		GP_BUG("Invalid size %i", len);
	}
}

int GP_FRead(FILE *f, const char *fmt, ...)
{
	int type, val, ret = 0;
	void *ptr;
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
			ptr = va_arg(va, void*);

			if (fread(ptr, val, 1, f) != 1)
				goto end;
		
			swap_bytes(ptr, val, type);
		break;
		case IGNORE:
			while (val--)
				fgetc(f);
		break;
		default:
			GP_BUG("Wrong format type for reading (%i)", type);
			goto end;
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
	uint8_t  u8;
	uint16_t u16;
	uint32_t u32;

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
			case 1:
				u8 = va_arg(va, int);
				if (fwrite(&u8, 1, 1, f) != 1)
					goto end;
			break;
			case 2:
				u16 = va_arg(va, int);
				
				swap_bytes(&u16, 2, type);
				
				if (fwrite(&u16, 2, 1, f) != 1)
					goto end;
			break;
			case 4:
				u32 = va_arg(va, int);
				
				swap_bytes(&u32, 4, type);
				
				if (fwrite(&u32, 4, 1, f) != 1)
					goto end;
			break;
			}
		break;
		default:
			GP_BUG("Wrong format type for writing (%i)", type);
			goto end;
		}

		ret++;
	}
end:
	va_end(va);
	return ret;
}
