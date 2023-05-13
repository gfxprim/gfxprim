@ include source.t
/*
 * Blit conversions tests.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>

#include <core/gp_pixmap.h>
#include <core/gp_convert.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_blit.h>

#include "tst_test.h"

static void fill_pixmap(gp_pixmap *c, gp_pixel p)
{
	gp_coord x, y;

	for (x = 0; x < (gp_coord)c->w; x++)
		for (y = 0; y < (gp_coord)c->h; y++)
			gp_putpixel(c, x, y, p);
}

static void mess_pixmap(gp_pixmap *c)
{
	gp_coord y;
	unsigned int i;

	for (y = 0; y < (gp_coord)c->h; y++) {
		uint8_t *row = GP_PIXEL_ADDR(c, 0, y);
		for (i = 0; i < c->bytes_per_row; i++) {
			row[i] = y ^ i;
		}
	}
}

static int check_filled(gp_pixmap *c, gp_pixel p)
{
	gp_coord x, y;
	gp_pixel pc;

	for (x = 0; x < (gp_coord)c->w; x++) {
		for (y = 0; y < (gp_coord)c->h; y++) {
			pc = gp_getpixel(c, x, y);
			if (p != pc) {
				tst_msg("Pixels different %08x %08x", p, pc);
				return 1;
			}
		}
	}

	return 0;
}

static gp_pixel rgb_to_pixel(int r, int g, int b, gp_pixmap *c)
{
	if (gp_pixel_has_flags(c->pixel_type, GP_PIXEL_HAS_ALPHA))
		return gp_rgba_to_pixmap_pixel(r, g, b, 0xff, c);

	return gp_rgb_to_pixmap_pixel(r, g, b, c);
}

@ def gen_blit(name, r, g, b, pt1, pt2):
static int blit_{{ name }}_{{ pt1.name }}_to_{{ pt2.name }}(void)
{
	gp_pixmap *src = gp_pixmap_alloc(100, 100, GP_PIXEL_{{ pt1.name }});
	gp_pixmap *dst = gp_pixmap_alloc(100, 100, GP_PIXEL_{{ pt2.name }});

	if (src == NULL || dst == NULL) {
		gp_pixmap_free(src);
		gp_pixmap_free(dst);
		tst_msg("Malloc failed :(");
		return TST_UNTESTED;
	}

	/* Fill source with color, destination with pseudo random mess */
	gp_pixel pix_src = rgb_to_pixel({{ r }}, {{ g }}, {{ b }}, src);
	gp_pixel pix_dst = rgb_to_pixel({{ r }}, {{ g }}, {{ b }}, dst);

        tst_msg("pixel_src=%08x pixel_dst=%08x", pix_src, pix_dst);

	fill_pixmap(src, pix_src);
	mess_pixmap(dst);

	gp_blit(src, 0, 0, src->w, src->h, dst, 0, 0);

	if (check_filled(dst, pix_dst))
		return TST_FAILED;

	return TST_PASSED;
}

@ def blit_color(name, r, g, b):
@     for pt1 in pixeltypes:
@         if not pt1.is_unknown() and not pt1.is_palette():
@             for pt2 in pixeltypes:
@                 if not pt2.is_unknown() and not pt2.is_palette():
{@ gen_blit(name, r, g, b, pt1, pt2) @}
@ end
@
{@ blit_color('black', '0x00', '0x00', '0x00') @}
{@ blit_color('white', '0xff', '0xff', '0xff') @}

@ def blit_equal_pixel(name, r, g, b):
@     for pt1 in pixeltypes:
@         if not pt1.is_unknown() and not pt1.is_palette():
{@ gen_blit(name, r, g, b, pt1, pt1) @}
@ end
@
{@ blit_equal_pixel('equal_pixel', '0x0f', '0xff', '0x00') @}

@ def gen_blit2(name, r, g, b, pname1, pname2):
{@ gen_blit(name, r, g, b, pixeltypes_dict[pname1], pixeltypes_dict[pname2]) @}
@ end

{@ gen_blit2('red', '0xff', '0x00', '0x00', 'RGB888', 'CMYK8888') @}
{@ gen_blit2('green', '0x00', '0xff', '0x00', 'RGB888', 'CMYK8888') @}
{@ gen_blit2('blue', '0x00', '0x00', '0xff', 'RGB888', 'CMYK8888') @}
{@ gen_blit2('gray', '0xef', '0xef', '0xef', 'RGB888', 'CMYK8888') @}
{@ gen_blit2('red', '0xff', '0x00', '0x00', 'CMYK8888', 'RGB888') @}
{@ gen_blit2('green', '0x00', '0xff', '0x00', 'CMYK8888', 'RGB888') @}
{@ gen_blit2('blue', '0x00', '0x00', '0xff', 'CMYK8888', 'RGB888') @}
{@ gen_blit2('gray', '0xef', '0xef', '0xef', 'CMYK8888', 'RGB888') @}

@ def gen_suite_entry(name, p_from, p_to):
		{.name = "Blit {{ p_from }} to {{ p_to }}",
		 .tst_fn = blit_{{ name }}_{{ p_from }}_to_{{ p_to }}},
@ end

const struct tst_suite tst_suite = {
	.suite_name = "Blit Conversions Testsuite",
	.tests = {
@ for pt1 in pixeltypes:
@     if not pt1.is_unknown() and not pt1.is_palette():
@         for pt2 in pixeltypes:
@             if not pt2.is_unknown() and not pt2.is_palette():
		{.name = "Blit black {{ pt1.name }} to {{ pt2.name }}",
		 .tst_fn = blit_black_{{ pt1.name }}_to_{{ pt2.name }}},
		{.name = "Blit white {{ pt1.name }} to {{ pt2.name }}",
		 .tst_fn = blit_white_{{ pt1.name }}_to_{{ pt2.name }}},
@ for pt1 in pixeltypes:
@     if not pt1.is_unknown() and not pt1.is_palette():
		{.name = "Blit {{ pt1.name }} to {{ pt1.name }}",
		 .tst_fn = blit_equal_pixel_{{ pt1.name }}_to_{{ pt1.name }}},
@ end

{@ gen_suite_entry('red', 'RGB888', 'CMYK8888') @}
{@ gen_suite_entry('green', 'RGB888', 'CMYK8888') @}
{@ gen_suite_entry('blue', 'RGB888', 'CMYK8888') @}
{@ gen_suite_entry('gray', 'RGB888', 'CMYK8888') @}
{@ gen_suite_entry('red', 'CMYK8888', 'RGB888') @}
{@ gen_suite_entry('green', 'CMYK8888', 'RGB888') @}
{@ gen_suite_entry('blue', 'CMYK8888', 'RGB888') @}
{@ gen_suite_entry('gray', 'CMYK8888', 'RGB888') @}

		{.name = NULL}
	}
};
