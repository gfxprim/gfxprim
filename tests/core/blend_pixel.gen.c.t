@ include source.t
/*
 * gp_blend_pixel() tests.
 *
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * gp_blend_pixel() composites the source over the destination in the native
 * channel depths of a pixel type, so it is checked for every pixel type with
 * an alpha channel against the same formula evaluated in floating point:
 *
 *  oa = sa + da * (1 - sa)
 *  c  = (sc * sa + dc * da * (1 - sa)) / oa
 *
 * Pixel types up to 8 bits are checked for every pair of pixels, the rest for
 * every pair of alpha values with a handful of colors.
 *
 * Pixel types without an alpha channel have an opaque source, which has to
 * be returned as it is.
 */

#include <math.h>
#include <core/gp_pixel.h>
#include <core/gp_blend_pixel.gen.h>

#include "tst_test.h"

/* Channel values, distinct for each channel, so that a swap shows up. */
static const unsigned int colors[][3] = {
	{0x00, 0x00, 0x00},
	{0xffff, 0xffff, 0xffff},
	{0x12, 0x80, 0xff},
	{0xff, 0x34, 0x00},
	{0x7f, 0xc0, 0x01},
};

#define COLORS (sizeof(colors) / sizeof(*colors))

@ for pt in pixeltypes:
@     if pt.is_alpha():
@         A = pt.chans['A']
@         chans = [c for c in pt.chanslist if not c.is_alpha]
static int check_{{ pt.name }}(gp_pixel src, gp_pixel dst)
{
	gp_pixel res = gp_blend_pixel(src, dst, GP_PIXEL_{{ pt.name }});
	unsigned int sa = GP_PIXEL_GET_A_{{ pt.name }}(src);
	unsigned int da = GP_PIXEL_GET_A_{{ pt.name }}(dst);
	double fsa = sa / {{ A.max }}.0;
	double fda = da / {{ A.max }}.0 * (1 - fsa);
	double foa = fsa + fda;
	double ref;

	if (res != gp_blend_pixel_{{ pt.name }}(src, dst)) {
		tst_msg("gp_blend_pixel() does not dispatch to gp_blend_pixel_{{ pt.name }}()");
		return 1;
	}

	if (sa == 0) {
		if (res == dst)
			return 0;

		tst_msg("transparent src 0x%08x over 0x%08x = 0x%08x",
		        src, dst, res);
		return 1;
	}

	if (sa == {{ A.C_max }} || da == 0) {
		if (res == src)
			return 0;

		tst_msg("src 0x%08x over 0x%08x = 0x%08x expected src",
		        src, dst, res);
		return 1;
	}

	if (da == {{ A.C_max }} && GP_PIXEL_GET_A_{{ pt.name }}(res) != {{ A.C_max }}) {
		tst_msg("src 0x%08x over opaque 0x%08x = 0x%08x not opaque",
		        src, dst, res);
		return 1;
	}

	ref = foa * {{ A.max }};
	if (fabs(GP_PIXEL_GET_A_{{ pt.name }}(res) - ref) > 0.5 + 1e-9) {
		tst_msg("src 0x%08x over 0x%08x = 0x%08x A expected %.3f",
		        src, dst, res, ref);
		return 1;
	}

@         for c in chans:
	ref = (GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(src) * fsa +
	       GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(dst) * fda) / foa;
	if (fabs(GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(res) - ref) > 0.5 + 1e-9) {
		tst_msg("src 0x%08x over 0x%08x = 0x%08x {{ c.name }} expected %.3f",
		        src, dst, res, ref);
		return 1;
	}

@         end
	return 0;
}

static int blend_{{ pt.name }}(void)
{
	unsigned long fail = 0;
@         if pt.pixelpack.size <= 8:
	gp_pixel src, dst;

	for (src = 0; src <= {{ hex(2 ** pt.pixelpack.size - 1) }}; src++) {
		for (dst = 0; dst <= {{ hex(2 ** pt.pixelpack.size - 1) }}; dst++) {
			fail += check_{{ pt.name }}(src, dst);
		}
	}
@         else:
	unsigned int sa, da, sc, dc;

	for (sa = 0; sa <= {{ A.C_max }}; sa++) {
		for (da = 0; da <= {{ A.C_max }}; da++) {
			for (sc = 0; sc < COLORS; sc++) {
				for (dc = 0; dc < COLORS; dc++) {
@             src_args = []
@             dst_args = []
@             for c in pt.chanslist:
@                 if c.is_alpha:
@                     src_args.append('sa')
@                     dst_args.append('da')
@                 else:
@                     i = chans.index(c)
@                     src_args.append('colors[sc][%i] & %s' % (i, c.C_max))
@                     dst_args.append('colors[dc][%i] & %s' % (i, c.C_max))
@                 end
@             end
					gp_pixel src = GP_PIXEL_CREATE_{{ pt.name }}({{ ', '.join(src_args) }});
					gp_pixel dst = GP_PIXEL_CREATE_{{ pt.name }}({{ ', '.join(dst_args) }});

					fail += check_{{ pt.name }}(src, dst);
				}
			}
		}
	}
@         end

	return fail ? TST_FAILED : TST_PASSED;
}

@ end
static const gp_pixel pixels[] = {
	0x00000000, 0xffffffff, 0x12345678, 0x87654321, 0x00ff00ff,
};

static unsigned long check_src(gp_pixel_type type, gp_pixel mask)
{
	unsigned int i, j;
	unsigned long f = 0;

	for (i = 0; i < GP_ARRAY_SIZE(pixels); i++) {
		for (j = 0; j < GP_ARRAY_SIZE(pixels); j++) {
			gp_pixel src = pixels[i] & mask;
			gp_pixel dst = pixels[j] & mask;
			gp_pixel res = gp_blend_pixel(src, dst, type);

			if (res == src)
				continue;

			tst_msg("%s src 0x%08x over 0x%08x = 0x%08x expected src",
			        gp_pixel_type_name(type), src, dst, res);
			f++;
		}
	}

	return f;
}

static int blend_no_alpha(void)
{
	unsigned long fail = 0;

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_alpha():
	fail += check_src(GP_PIXEL_{{ pt.name }}, {{ hex(2 ** pt.pixelpack.size - 1) }});
@ end

	return fail ? TST_FAILED : TST_PASSED;
}

const struct tst_suite tst_suite = {
	.suite_name = "Blend Pixel Testsuite",
	.tests = {
@ for pt in pixeltypes:
@     if pt.is_alpha():
		{.name = "Blend {{ pt.name }}", .tst_fn = blend_{{ pt.name }}},
@ end
		{.name = "Blend pixel types without alpha", .tst_fn = blend_no_alpha},
		{.name = NULL}
	}
};
