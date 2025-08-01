// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>

#include <core/gp_debug.h>
#include <core/gp_byte_order.h>
#include <utils/gp_utf.h>
#include <loaders/gp_io_parser.h>

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
