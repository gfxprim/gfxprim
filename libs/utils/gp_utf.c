// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <stddef.h>
#include <utils/gp_utf.h>

int8_t gp_utf8_next_chsz(const char *str, size_t off)
{
	char ch = str[off];
	uint8_t len = 0;

	if (!ch)
		return 0;

	if (GP_UTF8_IS_ASCII(ch))
		return 1;

	if (GP_UTF8_IS_2BYTE(ch)) {
		len = 2;
		goto ret;
	}

	if (GP_UTF8_IS_3BYTE(ch)) {
		len = 3;
		goto ret;
	}

	if (GP_UTF8_IS_4BYTE(ch)) {
		len = 4;
		goto ret;
	}

	return -1;
ret:
	if (!GP_UTF8_IS_NBYTE(str[off+1]))
		return -1;

	if (len > 2 && !GP_UTF8_IS_NBYTE(str[off+2]))
		return -1;

	if (len > 3 && !GP_UTF8_IS_NBYTE(str[off+3]))
		return -1;

	return len;
}

int8_t gp_utf8_prev_chsz(const char *str, size_t off)
{
	char ch = str[off];

	if (GP_UTF8_IS_ASCII(ch))
		return 1;

	if (!GP_UTF8_IS_NBYTE(ch))
		return -1;

	if (off < 1)
		return -1;

	ch = str[--off];

	if (GP_UTF8_IS_2BYTE(ch))
		return 2;

	if (!GP_UTF8_IS_NBYTE(ch))
		return -1;

	if (off < 1)
		return -1;

	ch = str[--off];

	if (GP_UTF8_IS_3BYTE(ch))
		return 3;

	if (!GP_UTF8_IS_NBYTE(ch))
		return -1;

	if (off < 1)
		return -1;

	ch = str[--off];

	if (GP_UTF8_IS_4BYTE(ch))
		return 4;

	return -1;
}

size_t gp_utf8_strlen(const char *str)
{
	size_t cnt = 0;

	while (gp_utf8_next(&str))
		cnt++;

	return cnt;
}

static uint32_t strip_diacritics(uint32_t ch)
{
	/* Latin supplement */
	switch (ch) {
	case 0xc0 ... 0xc5:
		return 'A';
	case 0xc7:
		return 'C';
	case 0xc8 ... 0xcb:
		return 'E';
	case 0xcc ... 0xcf:
		return 'I';
	case 0xd1:
		return 'N';
	case 0xd2 ... 0xd6:
	case 0xd8:
		return 'O';
	case 0xd9 ... 0xdc:
		return 'U';
	case 0xdd:
		return 'Y';
	/* map sharp s to s */
	case 0xdf:
		return 's';
	case 0xe0 ... 0xe5:
		return 'a';
	case 0xe7:
		return 'c';
	case 0xe8 ... 0xeb:
		return 'e';
	case 0xec ... 0xef:
		return 'i';
	case 0xf1:
		return 'n';
	case 0xf2 ... 0xf6:
	case 0xf8:
		return 'o';
	case 0xf9 ... 0xfc:
		return 'u';
	case 0xfd ... 0xff:
		return 'y';
	}

	/* Latin extended A */
	switch (ch) {
	case 0x100:
	case 0x102:
	case 0x104:
		return 'A';
	case 0x101:
	case 0x103:
	case 0x105:
		return 'a';
	case 0x106:
	case 0x108:
	case 0x10a:
	case 0x10c:
		return 'C';
	case 0x107:
	case 0x109:
	case 0x10b:
	case 0x10d:
		return 'c';
	case 0x10e:
	case 0x110:
		return 'D';
	case 0x10f:
	case 0x111:
		return 'd';
	case 0x112:
	case 0x114:
	case 0x116:
	case 0x118:
	case 0x11a:
		return 'E';
	case 0x113:
	case 0x115:
	case 0x117:
	case 0x119:
	case 0x11b:
		return 'e';
	case 0x11c:
	case 0x11e:
	case 0x120:
	case 0x122:
		return 'G';
	case 0x11d:
	case 0x11f:
	case 0x121:
	case 0x123:
		return 'g';
	case 0x124:
	case 0x126:
		return 'H';
	case 0x125:
	case 0x127:
		return 'h';
	case 0x128:
	case 0x12a:
	case 0x12c:
	case 0x12e:
	case 0x130:
		return 'I';
	case 0x129:
	case 0x12b:
	case 0x12d:
	case 0x12f:
	case 0x131:
		return 'i';
	case 0x134:
		return 'J';
	case 0x135:
		return 'j';
	case 0x136:
		return 'K';
	case 0x137:
		return 'k';
	case 0x139:
	case 0x13b:
	case 0x13d:
	case 0x13f:
	case 0x141:
		return 'L';
	case 0x13a:
	case 0x13c:
	case 0x13e:
	case 0x140:
	case 0x142:
		return 'l';
	case 0x143:
	case 0x145:
	case 0x147:
		return 'N';
	case 0x144:
	case 0x146:
	case 0x148:
		return 'n';
	case 0x14c:
	case 0x14e:
	case 0x150:
		return 'O';
	case 0x14d:
	case 0x14f:
	case 0x151:
		return 'o';
	case 0x154:
	case 0x156:
	case 0x158:
		return 'R';
	case 0x155:
	case 0x157:
	case 0x159:
		return 'r';
	case 0x15a:
	case 0x15c:
	case 0x15e:
	case 0x160:
		return 'S';
	case 0x15b:
	case 0x15d:
	case 0x15f:
	case 0x161:
		return 's';
	case 0x162:
	case 0x164:
	case 0x166:
		return 'T';
	case 0x163:
	case 0x165:
	case 0x167:
		return 't';
	case 0x168:
	case 0x16a:
	case 0x16c:
	case 0x16e:
	case 0x170:
	case 0x172:
		return 'U';
	case 0x169:
	case 0x16b:
	case 0x16d:
	case 0x16f:
	case 0x171:
	case 0x173:
		return 'u';
	case 0x174:
		return 'W';
	case 0x175:
		return 'w';
	case 0x176:
	case 0x178:
		return 'Y';
	case 0x177:
		return 'y';
	case 0x179:
	case 0x17b:
	case 0x17d:
		return 'Z';
	case 0x17a:
	case 0x17c:
	case 0x17e:
		return 'z';
	}

	return ch;
}

uint32_t gp_utf_fallback(uint32_t ch)
{
	uint32_t ret;

	ret = strip_diacritics(ch);
	if (ret != ch)
		return ret;

	return ch;
}
