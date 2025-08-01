// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>

#include <core/gp_byte_order.h>
#include <core/gp_debug.h>
#include <core/gp_common.h>

#include <utils/gp_utf.h>

#include <loaders/gp_io.h>
#include <loaders/gp_io_parser.h>

#define TYPE(x) ((x) & GP_IO_TYPE_MASK)
#define VAL(x) ((x) & ~GP_IO_TYPE_MASK)

static void readf_size(const uint16_t *types,
                       unsigned int *min_size, unsigned int *max_size)
{
	unsigned int min = 0;
	unsigned int max = 0;

	while (*types != GP_IO_END) {
		switch (TYPE(*types)) {
		case GP_IO_CONST:
		case GP_IO_BYTE:
			min++;
			max++;
		break;
		case GP_IO_L2:
		case GP_IO_B2:
			min += 2;
			max += 2;
		break;
		case GP_IO_L4:
		case GP_IO_B4:
			min += 4;
			max += 4;
		break;
		case GP_IO_ARRAY:
		case GP_IO_IGN:
			min += VAL(*types);
			max += VAL(*types);
		break;
		case GP_IO_PPSTR:
			min += 2;
			max += 255;
		break;
		}
		types++;
	}

	*min_size = min;
	*max_size = max;
}

static int needs_swap(uint16_t type)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	if (type == GP_IO_L2 || type == GP_IO_L4)
		return 1;
	return 0;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	if (type == GP_IO_B2 || type == GP_IO_B4)
		return 1;
	return 0;
#else
# error Unknown Endianity
#endif
}

static void write_str(uint16_t type, uint8_t *dest,
                      uint8_t *src, unsigned int size)
{
	unsigned int dest_size = VAL(type);
	unsigned int i;

	if (!dest_size)
		return;

	for (i = 0; i < dest_size - 1 && i < size; i++)
		dest[i] = src[i];

	dest[i] = '\0';
}

int gp_io_readf(gp_io *self, const uint16_t *types, ...)
{
	unsigned int read_size, buf_size, size;
	int ret;
	va_list va;
	uint8_t *ptr;

	readf_size(types, &read_size, &buf_size);

	if (!read_size)
		return 0;

	uint8_t buffer[buf_size], *buf = buffer;

	if (gp_io_fill(self, buf, read_size))
		return -1;

	ret = 0;
	va_start(va, types);

	while (*types != GP_IO_END) {
		switch (TYPE(*types)) {
		case GP_IO_CONST:
			//TODO: Seek back?
			if (VAL(*types) != *buf) {
				errno = EINVAL;
				goto end;
			}
			buf++;
		break;
		case GP_IO_BYTE:
			ptr = va_arg(va, uint8_t*);
			*ptr = *buf;
			buf++;
		break;
		case GP_IO_L2:
		case GP_IO_B2:
			ptr = va_arg(va, uint8_t*);

			if (needs_swap(*types)) {
				ptr[0] = buf[1];
				ptr[1] = buf[0];
			} else {
				ptr[0] = buf[0];
				ptr[1] = buf[1];
			}
			buf += 2;
		break;
		case GP_IO_L4:
		case GP_IO_B4:
			ptr = va_arg(va, uint8_t*);

			if (needs_swap(*types)) {
				ptr[0] = buf[3];
				ptr[1] = buf[2];
				ptr[2] = buf[1];
				ptr[3] = buf[0];
			} else {
				ptr[0] = buf[0];
				ptr[1] = buf[1];
				ptr[2] = buf[2];
				ptr[3] = buf[3];
			}
			buf += 4;
		break;
		case GP_IO_ARRAY:
			ptr = va_arg(va, void*);
			memcpy(ptr, buf, VAL(*types));
		/* fallthrough */
		case GP_IO_IGN:
			buf += VAL(*types);
		break;
		case GP_IO_PPSTR:
			ptr = va_arg(va, void*);
			size = *buf;

			/* empty string */
			if (!size) {
				write_str(*types, ptr, NULL, 0);
				buf += 2;
			} else {
				/* fill up another part of the buffer */
				if (gp_io_fill(self, buf + read_size, size))
					return -1;
				read_size += size;
				write_str(*types, ptr, buf + 1, size);
				buf += GP_ALIGN2(size + 1);
                        }
		break;
		}

		types++;
		ret++;
	}

end:
	va_end(va);
	return ret;
}

int gp_io_writef(gp_io *io, const uint16_t *types, ...)
{
	va_list va;
	uint8_t *ptr, t;
	int32_t i4;
	int16_t i2;

	va_start(va, types);

	while (*types != GP_IO_END) {
		switch (TYPE(*types)) {
		case GP_IO_CONST:
			t = VAL(*types);
			if (gp_io_write(io, &t, 1) != 1)
				goto err;
		break;
		case GP_IO_L2:
		case GP_IO_B2:
			i2 = va_arg(va, int);
			ptr = (void*)&i2;

			if (needs_swap(*types))
				GP_SWAP(ptr[0], ptr[1]);

			if (gp_io_write(io, ptr, 2) != 2)
				goto err;
		break;
		case GP_IO_L4:
		case GP_IO_B4:
			i4 = va_arg(va, int);
			ptr = (void*)&i4;

			if (needs_swap(*types)) {
				GP_SWAP(ptr[0], ptr[3]);
				GP_SWAP(ptr[1], ptr[2]);
			}

			if (gp_io_write(io, ptr, 4) != 4)
				goto err;
		break;
		default:
			GP_WARN("Invalid type %"PRIu16"\n", *types);
			goto err;
		}
		types++;
	}

	va_end(va);
	return 0;
err:
	va_end(va);
	return -1;
}

int gp_io_read_b4(gp_io *io, uint32_t *val)
{
	uint16_t desc[] = {
		GP_IO_B4,
		GP_IO_END
	};

	return gp_io_readf(io, desc, val) != 1;
}

int gp_io_read_b2(gp_io *io, uint16_t *val)
{
	uint16_t desc[] = {
		GP_IO_B2,
		GP_IO_END
	};

	return gp_io_readf(io, desc, val) != 1;
}

char *gp_io_read_b2_utf16(gp_io *io, size_t nchars)
{
	uint16_t *chars = malloc(nchars * 2);

	if (!chars) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	if (gp_io_fill(io, chars, nchars * 2)) {
		GP_WARN("Failed to read bytes");
		return NULL;
	}

#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint16_t *tmp = chars;
	size_t i;

	for (i = 0; i < nchars; i++) {
		uint16_t c = *tmp;
		*tmp = ((0xff00 & c)>>8) | ((0x00ff & c)<<8);
		tmp++;
	}
#endif

	size_t utf8_size = gp_utf16_to_utf8_size(chars, nchars);

	if (!utf8_size) {
		GP_WARN("Length overflow");
		return NULL;
	}

	char *utf8_str = malloc(utf8_size);

	if (!utf8_str) {
		free(chars);
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	gp_utf16_to_utf8(chars, nchars, utf8_str);

	free(chars);

	return utf8_str;
}
