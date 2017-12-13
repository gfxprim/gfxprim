@ include source.t
/*
 * Pixel conversions tests.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */
#include <stdio.h>
#include <core/GP_Convert.h>

#include "tst_test.h"

/*
 * Returns black color for particular pixel type.
 */
static gp_pixel get_black(gp_pixel_type pixel_type)
{
	switch (pixel_type) {
@ for pt in pixeltypes:
	case {{ pt.C_type }}:
@     if pt.is_cmyk():
@         K = pt.chans['K']
		/* Black in CMYK is full K rest zero */
		return {{ K.C_mask }};
@     elif pt.is_alpha():
@         A = pt.chans['A']
		/* Black with Alpha channel is full A rest zero */
		return {{ A.C_mask }};
@     else:
		return 0;
@ end
	default:
		tst_msg("Invalid pixel type %i", pixel_type);
		exit(TST_INTERR);
	}
}

/*
 * Returns white color for particular pixel type.
 */
static gp_pixel get_white(gp_pixel_type pixel_type)
{
	switch (pixel_type) {
@ for pt in pixeltypes:
	case {{ pt.C_type }}:
@     if pt.is_cmyk():
		/* White in CMYK is zero */
		return 0x0;
@     elif pt.is_rgb():
@         R = pt.chans['R']
@         G = pt.chans['G']
@         B = pt.chans['B']
@         if pt.is_alpha():
@             A = pt.chans['A']
		/* White in RGBA */
		return {{ A.C_mask }} | {{ R.C_mask }} | {{ G.C_mask }} | {{ B.C_mask }};
@         else:
		/* Plain old RGB */
		return {{ R.C_mask }} | {{ G.C_mask }} | {{ B.C_mask }};
@         end
@     elif pt.is_gray():
@         V = pt.chans['V']
@         if pt.is_alpha():
@             A = pt.chans['A']
		/* Grayscale with Alpha */
		return {{ V.C_mask }} | {{ A.C_mask }};
@         else:
		/* Grayscale */
		return {{ V.C_mask }};
@         end
@     else:
		tst_msg("FIXME: Unsupported conversion to %s",
		        gp_pixel_type_name(pixel_type));
		exit(TST_INTERR);
@ end
	default:
		tst_msg("Invalid pixel type %i", pixel_type);
		exit(TST_INTERR);
	}
}

/*
 * Returns red color for particular pixel type.
 */
static gp_pixel get_red(gp_pixel_type pixel_type)
{
	switch (pixel_type) {
@ for pt in pixeltypes:
	case {{ pt.C_type }}:
@     if pt.is_cmyk():
@         M = pt.chans['M']
@         Y = pt.chans['Y']
		/* Red in CMYK is full M and Y rest zero */
		return {{ M.C_mask }} | {{ Y.C_mask }};
@     elif pt.is_rgb():
@         R = pt.chans['R']
@         if pt.is_alpha():
@             A = pt.chans['A']
		/* Red with Alpha channel is full Alpha and R rest zero */
		return {{ A.C_mask }} | {{ R.C_mask }};
@         else:
		/* Plain old RGB */
		return {{ R.C_mask }};
@         end
@     elif pt.is_gray():
@         V = pt.chans['V']
@         if pt.is_alpha():
@             A = pt.chans['A']
		/* Grayscale with Alpha channel is full Alpha + 1/3 Gray */
		return ({{ hex(V.max // 3)}}{{ V.C_shift }}) | {{ A.C_mask }};
@         else:
		/* Grayscale is 1/3 Gray */
		return {{ hex(V.max // 3) }}{{ V.C_shift }};
@         end
@     else:
		tst_msg("FIXME: Unsupported conversion to %s",
		        gp_pixel_type_name(pixel_type));
		exit(TST_INTERR);
@ end
	default:
		tst_msg("Invalid pixel type %i", pixel_type);
		exit(TST_INTERR);
	}
}

@ def gen_convert_and_check(test_name, in_name, out_name):
static int convert_and_check_{{ test_name }}_{{ in_name }}_to_{{ out_name }}(void)
{
	gp_pixel out = 0;
	gp_pixel in = get_{{ test_name }}(GP_PIXEL_{{ in_name }});
	gp_pixel out_exp = get_{{ test_name }}(GP_PIXEL_{{ out_name }});

	tst_msg("{{ in_name }} %08x -> {{ out_name }} %08x", in, out_exp);

	GP_PIXEL_{{ in_name }}_TO_{{ out_name }}(in, out);

	if (out_exp != out) {
		tst_msg("Pixels are different have %08x, expected %08x",
		        out, out_exp);
		return TST_FAILED;
	}

	return TST_SUCCESS;
}

@ end
@
@ def gen_converts():
@     for pt1 in pixeltypes:
@         if not pt1.is_unknown() and not pt1.is_palette():
@             if pt1.name not in ['RGB888', 'RGBA8888']:
@ # White
{@ gen_convert_and_check('white', pt1.name, 'RGB888') @}
{@ gen_convert_and_check('white', pt1.name, 'RGBA8888') @}
{@ gen_convert_and_check('white', 'RGB888', pt1.name) @}
{@ gen_convert_and_check('white', 'RGBA8888', pt1.name) @}
@ # Black
{@ gen_convert_and_check('black', pt1.name, 'RGB888') @}
{@ gen_convert_and_check('black', pt1.name, 'RGBA8888') @}
{@ gen_convert_and_check('black', 'RGB888', pt1.name) @}
{@ gen_convert_and_check('black', 'RGBA8888', pt1.name) @}
@ # Red
@                 if not pt1.is_gray():
{@ gen_convert_and_check('red', pt1.name, 'RGB888') @}
{@ gen_convert_and_check('red', pt1.name, 'RGBA8888') @}
@                 end
{@ gen_convert_and_check('red', 'RGB888', pt1.name) @}
{@ gen_convert_and_check('red', 'RGBA8888', pt1.name) @}
@ end
@
{@ gen_converts() @}

@ def gen_suite_entry(name, p_from, p_to):
		{.name = "Convert {{ name }} {{ p_from }} -> {{ p_to }}",
		 .tst_fn = convert_and_check_{{ name }}_{{ p_from }}_to_{{ p_to }}},
@ end

const struct tst_suite tst_suite = {
	.suite_name = "Pixel Conversions Testsuite",
	.tests = {
@ for pt1 in pixeltypes:
@     if not pt1.is_unknown() and not pt1.is_palette():
@         if pt1.name not in ['RGB888', 'RGBA8888']:
@ # White
{@ gen_suite_entry('white', pt1.name, 'RGB888') @}
{@ gen_suite_entry('white', pt1.name, 'RGBA8888') @}
{@ gen_suite_entry('white', 'RGB888', pt1.name) @}
{@ gen_suite_entry('white', 'RGBA8888', pt1.name) @}
@ # Black
{@ gen_suite_entry('black', pt1.name, 'RGB888') @}
{@ gen_suite_entry('black', pt1.name, 'RGBA8888') @}
{@ gen_suite_entry('black', 'RGB888', pt1.name) @}
{@ gen_suite_entry('black', 'RGBA8888', pt1.name) @}
@ # Red
@             if not pt1.is_gray():
{@ gen_suite_entry('red', pt1.name, 'RGB888') @}
{@ gen_suite_entry('red', pt1.name, 'RGBA8888') @}
@             end
{@ gen_suite_entry('red', 'RGB888', pt1.name) @}
{@ gen_suite_entry('red', 'RGBA8888', pt1.name) @}
@ end
		{.name = NULL}
	}
};
