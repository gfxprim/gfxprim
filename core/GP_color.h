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

#ifndef GP_COLOR_H
#define GP_COLOR_H

#include <stdint.h>

#define GP_COLNAME_PACK(n) {.colname = {GP_COLNAME, n}}
#define GP_G1_PACK(g) {.colname = {GP_G1, g}}
#define GP_G2_PACK(g) {.colname = {GP_G2, g}}
#define GP_G4_PACK(g) {.colname = {GP_G4, g}}
#define GP_G8_PACK(g) {.colname = {GP_G8, g}}
#define GP_RGB555_PACK(r, g, b) {.rgb555 = {GP_RGB555, r, g, b}}
#define GP_RGB888_PACK(r, g, b) {.rgb888 = {GP_RGB888, r, g, b}}
#define GP_RGBA8888_PACK(r, g, b, a) {.rgb888 = {GP_RGBA8888, r, g, b, a}}

#define GP_COLNAME_FILL(col, n) do {   \
	(col)->name.type = GP_COLNAME; \
	(col)->name.name = n;          \
} while (0)

#define GP_G1_FILL(col, g) do { \
	(col)->g1.type = GP_G1; \
	(col)->g1.gray = g;     \
} while (0)

#define GP_G2_FILL(col, g) do { \
	(col)->g2.type = GP_G2; \
	(col)->g2.gray = g;     \
} while (0)

#define GP_G4_FILL(col, g) do { \
	(col)->g4.type = GP_G4; \
	(col)->g4.gray = g;     \
} while (0)

#define GP_G8_FILL(col, g) do { \
	(col)->g8.type = GP_G8; \
	(col)->g8.gray = g;     \
} while (0)

#define GP_RGB555_FILL(col, r, g, b) do {\
	(col)->rgb555.type  = GP_RGB555; \
	(col)->rgb555.red   = r;         \
	(col)->rgb555.green = g;         \
	(col)->rgb555.blue  = b;         \
} while (0)

#define GP_RGB888_FILL(col, r, g, b) do {\
	(col)->rgb888.type  = GP_RGB888; \
	(col)->rgb888.red   = r;         \
	(col)->rgb888.green = g;         \
	(col)->rgb888.blue  = b;         \
} while (0)

#define GP_RGBA8888_FILL(col, r, g, b, a) do {\
	(col)->rgba8888.type  = GP_RGBA8888;  \
	(col)->rgba8888.red   = r;            \
	(col)->rgba8888.green = g;            \
	(col)->rgba8888.blue  = b;            \
	(col)->rgba8888.alpha = a;            \
} while (0)

typedef enum GP_ColorName {
	GP_COL_BLACK,
	GP_COL_RED,
	GP_COL_GREEN,
	GP_COL_BLUE,
	GP_COL_YELLOW,
	GP_COL_BROWN,
	GP_COL_ORANGE,
	GP_COL_GRAY1,
	GP_COL_GRAY2,
	GP_COL_PURPLE,
	GP_COL_WHITE,
	GP_COL_MAX,
} GP_ColorName;

typedef enum GP_ColorType {
	GP_COLNAME,
	GP_G1,
	GP_G2,
	GP_G4,
	GP_G8,
	GP_RGB555,
	GP_RGB888,
	GP_RGBA8888,
	GP_COLMAX,
} GP_ColorType;

struct GP_ColName {
	enum GP_ColorType type;
	enum GP_ColorName name;
};

struct GP_ColRGB888 {
	enum GP_ColorType type;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct GP_ColG1 {
	enum GP_ColorType type;
	uint8_t gray:1;
};

struct GP_ColG2 {
	enum GP_ColorType type;
	uint8_t gray:2;
};

struct GP_ColG4 {
	enum GP_ColorType type;
	uint8_t gray:4;
};

struct GP_ColG8 {
	enum GP_ColorType type;
	uint8_t gray;
};

struct GP_ColRGBA8888 {
	enum GP_ColorType type;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

struct GP_ColRGB555 {
	enum GP_ColorType type;
	uint16_t red:5;
	uint16_t green:5;
	uint16_t blue:5;
};

typedef union GP_Col {
	enum GP_ColorType     type;
	struct GP_ColName     colname;
	struct GP_ColG1       g1;
	struct GP_ColG2       g2;
	struct GP_ColG4       g4;
	struct GP_ColG8       g8;
	struct GP_ColRGB888   rgb888;
	struct GP_ColRGB555   rgb555;
	struct GP_ColRGBA8888 rgba8888;
} GP_Color;

enum GP_RetCode {
	GP_ESUCCESS,
	GP_EINVAL,
	GP_ENOIMPL,
	GP_EUNPRECISE,
};

/*
 * Convers color pointed by *color to type.
 */
enum GP_RetCode GP_ColorConvert(GP_Color *color, GP_ColorType type);

#endif
