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

 /*

   Color to pixel conversion test.

  */

#include <GP.h>

#define RED   0x0c
#define GREEN 0x0a
#define BLUE  0x01
#define ALPHA 0x31

#define RGB555 0x3141 
#define BGR555 0x054c

#define XRGB 0x000c0a01
#define XBGR 0x00010a0c
#define RGBX 0x0c0a0100
#define BGRX 0x010a0c00

#define RGBA 0x0c0a0131
#define ARGB 0x310c0a01
#define BGRA 0x010a0c31
#define ABGR 0x31010a0c

struct test_data  {
	GP_Color     color;
	GP_PixelType type;
	GP_Pixel     pixel;
};

struct test_data test_data[] = {
	/* RGB555 */
	{GP_RGB555_PACK(RED, GREEN, BLUE), GP_PIXEL_RGB555, {.v16 = RGB555}},
	{GP_RGB555_PACK(RED, GREEN, BLUE), GP_PIXEL_BGR555, {.v16 = BGR555}},
	/* RGB888 */
	{GP_RGB888_PACK(RED, GREEN, BLUE), GP_PIXEL_RGB888, {.v32 = XRGB}},
	{GP_RGB888_PACK(RED, GREEN, BLUE), GP_PIXEL_BGR888, {.v32 = XBGR}},
	{GP_RGB888_PACK(RED, GREEN, BLUE), GP_PIXEL_XBGR8888, {.v32 = XBGR}},
	{GP_RGB888_PACK(RED, GREEN, BLUE), GP_PIXEL_XRGB8888, {.v32 = XRGB}},
	{GP_RGB888_PACK(RED, GREEN, BLUE), GP_PIXEL_RGBX8888, {.v32 = RGBX}},
	{GP_RGB888_PACK(RED, GREEN, BLUE), GP_PIXEL_BGRX8888, {.v32 = BGRX}},
	/* RGBA8888 */
	{GP_RGBA8888_PACK(RED, GREEN, BLUE, ALPHA), GP_PIXEL_RGBA8888, {.v32 = RGBA}},
	{GP_RGBA8888_PACK(RED, GREEN, BLUE, ALPHA), GP_PIXEL_ARGB8888, {.v32 = ARGB}},
	{GP_RGBA8888_PACK(RED, GREEN, BLUE, ALPHA), GP_PIXEL_BGRA8888, {.v32 = BGRA}},
	{GP_RGBA8888_PACK(RED, GREEN, BLUE, ALPHA), GP_PIXEL_ABGR8888, {.v32 = ABGR}},
	/* RGBA -> RGB conversion */
	{GP_RGBA8888_PACK(RED, GREEN, BLUE, ALPHA), GP_PIXEL_RGBX8888, {.v32 = RGBX}},
	{GP_RGBA8888_PACK(RED, GREEN, BLUE, ALPHA), GP_PIXEL_XRGB8888, {.v32 = XRGB}},
	{GP_RGBA8888_PACK(RED, GREEN, BLUE, ALPHA), GP_PIXEL_BGRX8888, {.v32 = BGRX}},
	{GP_RGBA8888_PACK(RED, GREEN, BLUE, ALPHA), GP_PIXEL_XBGR8888, {.v32 = XBGR}},
};

static int pixel_compare(GP_Pixel *a, GP_Pixel *b, GP_PixelType type)
{
	uint32_t size = GP_PixelSize(type);

	if (size <= 8)
		return a->v8 == b->v8;
	
	if (size <= 16)
		return a->v16 == b->v16;

	return a->v32 == b->v32;
}

static void do_test(struct test_data data)
{
	GP_Pixel pixel;
	GP_RetCode ret;

	printf("Testing %s -> %s ... ", GP_ColorTypeName(data.color.type),
	                                GP_PixelTypeName(data.type));

	ret = GP_ColorToPixel(data.color, data.type, &pixel);

	switch (ret) {
		case GP_ESUCCESS:
			printf(" (Color conversion was sucessful.) ...");
		break;
		case GP_EUNPRECISE:
			printf(" (Color conversion wasn't precise.) ...");
		break;
		default:
			printf(" GP_ColorToPixel returned %s. FAILED\n",
			       GP_RetCodeName(ret));
			return;
	}

	if (pixel_compare(&pixel, &data.pixel, data.type))
		printf(" SUCCEEDED\n");
	else
		printf(" FAILED expected = 0x%04x have = 0x%04x\n",
		       data.pixel.v32, pixel.v32);
}

int main(void)
{
	unsigned int i, size = sizeof (test_data) / sizeof (struct test_data);

	for (i = 0; i < size; i++)
		do_test(test_data[i]);

	return 0;
}
