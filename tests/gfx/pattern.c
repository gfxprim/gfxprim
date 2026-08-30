// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * A pattern reaches the pixels two different ways: a horizontal run, which
 * gp_fill() and gp_hline() draw, goes through gp_pixel_pattern_row_init_*()
 * and gp_write_pixels_pattern_*(); gp_vline() goes through
 * gp_pixel_pattern_col_init_*() and gp_write_pixels_pattern_col().
 *
 * Draw the same area both ways and require the results to be identical,
 * pixel value for pixel value. For every pattern, in both bit orders, and for
 * the pixel types that carry an alpha channel with the color one.
 */

#include <string.h>

#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_pattern_fill.h>
#include <core/gp_fill.h>
#include <gfx/gp_hline.h>
#include <gfx/gp_vline.h>

#include "tst_test.h"

#define W 8
#define H 8

struct testcase {
	enum gp_pixel_pattern pattern;
	enum gp_pixel_pattern alpha_pattern;
	gp_pixel fg;
	gp_pixel bg;
	gp_pixel_type ptype;

	/* test_over() only */
	gp_pixel alpha_mask;
	unsigned int kept;
};

static void dump(gp_pixmap *fill, gp_pixmap *other, const char *name)
{
	gp_coord x, y;

	tst_msg("gp_fill          %s", name);

	for (y = 0; y < H; y++) {
		char a[W+1], b[W+1];

		for (x = 0; x < W; x++) {
			a[x] = "0123456789abcdef"[gp_getpixel(fill, x, y) & 0x0f];
			b[x] = "0123456789abcdef"[gp_getpixel(other, x, y) & 0x0f];
		}

		a[W] = b[W] = 0;

		tst_msg("%s         %s", a, b);
	}
}

static int compare(gp_pixmap *fill, gp_pixmap *other, const char *name)
{
	gp_coord x, y;
	int err = 0;

	for (y = 0; y < H; y++) {
		for (x = 0; x < W; x++) {
			if (gp_getpixel(fill, x, y) != gp_getpixel(other, x, y))
				err++;
		}
	}

	if (err) {
		tst_msg("%s: %i of %i pixels differ", name, err, W * H);
		dump(fill, other, name);
	}

	return err;
}

static int test_pattern(struct testcase *t)
{
	gp_pixmap *vline, *fill, *hline, *vshort;
	gp_pixel pixel = gp_pixel_pattern_alpha(t->fg, t->bg, t->pattern,
	                                        t->alpha_pattern);
	gp_coord x, y;
	int err = 0;

	vline = gp_pixmap_alloc(W, H, t->ptype);
	fill = gp_pixmap_alloc(W, H, t->ptype);
	hline = gp_pixmap_alloc(W, H, t->ptype);
	vshort = gp_pixmap_alloc(W, H, t->ptype);

	if (!vline || !fill || !hline || !vshort) {
		tst_err("Failed to allocate pixmaps");
		return TST_UNTESTED;
	}

	memset(vline->pixels, 0, vline->bytes_per_row * H);
	memset(fill->pixels, 0, fill->bytes_per_row * H);
	memset(hline->pixels, 0, hline->bytes_per_row * H);
	memset(vshort->pixels, 0, vshort->bytes_per_row * H);

	gp_fill(fill, pixel);

	for (x = 0; x < W; x++)
		gp_vline(vline, x, 0, H - 1, pixel);

	/*
	 * Short runs of a length that does not divide the tile, so that they
	 * start at every column of it and in both halves of a byte.
	 */
	for (y = 0; y < H; y++) {
		gp_coord len = 1 + y % 5;

		for (x = 0; x < W; x += len)
			gp_hline(hline, x, GP_MIN(x + len, W) - 1, y, pixel);
	}

	/* And the same down the columns, starting at every row of the tile */
	for (x = 0; x < W; x++) {
		gp_coord len = 1 + x % 5;

		for (y = 0; y < H; y += len)
			gp_vline(vshort, x, y, GP_MIN(y + len, H) - 1, pixel);
	}

	err += compare(fill, vline, "vlines");
	err += compare(fill, hline, "hlines");
	err += compare(fill, vshort, "short vlines");

	gp_pixmap_free(vline);
	gp_pixmap_free(fill);
	gp_pixmap_free(hline);
	gp_pixmap_free(vshort);

	return err ? TST_FAILED : TST_PASSED;
}

/*
 * A pattern drawn over something must not clear the pixels it does not cover.
 *
 * A stipple used to write its transparent half, so drawing a translucent shape
 * over a drawing erased half of what was under it, and drawing a fully
 * transparent one erased all of it. A shape shaded with a gradient that runs
 * to a fully transparent stop is exactly that, and it punched holes through
 * whatever was drawn under it.
 *
 * So: fill the area with an opaque white, draw the pattern over it, and require
 * that what the pattern does not cover is still white. A pixel type with no
 * alpha channel has nothing to leave alone and must still cover everything.
 */
static int test_over(struct testcase *t)
{
	gp_pixmap *vline, *fill;
	gp_pixel pixel = gp_pixel_pattern_alpha(t->fg, t->bg, t->pattern,
	                                        t->alpha_pattern);
	gp_pixel keep = (1 << gp_pixel_size(t->ptype)) - 1;
	unsigned int kept = 0, cleared = 0;
	gp_coord x, y;
	int err = 0;

	vline = gp_pixmap_alloc(W, H, t->ptype);
	fill = gp_pixmap_alloc(W, H, t->ptype);

	if (!vline || !fill) {
		tst_err("Failed to allocate pixmaps");
		return TST_UNTESTED;
	}

	gp_fill(vline, keep);
	gp_fill(fill, keep);

	gp_fill(fill, pixel);

	for (x = 0; x < W; x++)
		gp_vline(vline, x, 0, H - 1, pixel);

	err += compare(fill, vline, "vlines");

	for (y = 0; y < H; y++) {
		for (x = 0; x < W; x++) {
			gp_pixel p = gp_getpixel(fill, x, y);

			if (p == keep)
				kept++;

			/* GA11 and GA22 alike: no alpha bit set at all. */
			if (t->alpha_mask && !(p & t->alpha_mask))
				cleared++;
		}
	}

	if (cleared) {
		tst_msg("%u pixels were cleared rather than left alone", cleared);
		err++;
	}

	if (kept != t->kept) {
		tst_msg("%u pixels kept, expected %u", kept, t->kept);
		err++;
	}

	gp_pixmap_free(vline);
	gp_pixmap_free(fill);

	return err ? TST_FAILED : TST_PASSED;
}

#define TESTCASE(nm, pat, pt) \
	static struct testcase tc_##nm = {pat, GP_PIXEL_PATTERN_FG, 1, 0, pt, 0, 0};

/* With the foreground and the background spelled out, and an alpha pattern. */
#define TESTCASE_EX(nm, pat, apat, fg, bg, pt) \
	static struct testcase tc_##nm = {pat, apat, fg, bg, pt, 0, 0};

TESTCASE(50_db, GP_PIXEL_PATTERN_50, GP_PIXEL_G1_DB)
TESTCASE(asc25_db, GP_PIXEL_PATTERN_ASC_25, GP_PIXEL_G1_DB)
TESTCASE(dsc25_db, GP_PIXEL_PATTERN_DSC_25, GP_PIXEL_G1_DB)
TESTCASE(asc75_db, GP_PIXEL_PATTERN_ASC_75, GP_PIXEL_G1_DB)
TESTCASE(dsc75_db, GP_PIXEL_PATTERN_DSC_75, GP_PIXEL_G1_DB)
TESTCASE(d625_db, GP_PIXEL_PATTERN_DOTS_6_25, GP_PIXEL_G1_DB)
TESTCASE(d125_db, GP_PIXEL_PATTERN_DOTS_12_5, GP_PIXEL_G1_DB)
TESTCASE(d25_db, GP_PIXEL_PATTERN_DOTS_25, GP_PIXEL_G1_DB)
TESTCASE(d75_db, GP_PIXEL_PATTERN_DOTS_75, GP_PIXEL_G1_DB)
TESTCASE(d875_db, GP_PIXEL_PATTERN_DOTS_87_5, GP_PIXEL_G1_DB)
TESTCASE(d9375_db, GP_PIXEL_PATTERN_DOTS_93_75, GP_PIXEL_G1_DB)

TESTCASE(50_ub, GP_PIXEL_PATTERN_50, GP_PIXEL_G1_UB)
TESTCASE(asc25_ub, GP_PIXEL_PATTERN_ASC_25, GP_PIXEL_G1_UB)
TESTCASE(dsc25_ub, GP_PIXEL_PATTERN_DSC_25, GP_PIXEL_G1_UB)
TESTCASE(asc75_ub, GP_PIXEL_PATTERN_ASC_75, GP_PIXEL_G1_UB)
TESTCASE(dsc75_ub, GP_PIXEL_PATTERN_DSC_75, GP_PIXEL_G1_UB)
TESTCASE(d625_ub, GP_PIXEL_PATTERN_DOTS_6_25, GP_PIXEL_G1_UB)
TESTCASE(d125_ub, GP_PIXEL_PATTERN_DOTS_12_5, GP_PIXEL_G1_UB)
TESTCASE(d25_ub, GP_PIXEL_PATTERN_DOTS_25, GP_PIXEL_G1_UB)
TESTCASE(d75_ub, GP_PIXEL_PATTERN_DOTS_75, GP_PIXEL_G1_UB)
TESTCASE(d875_ub, GP_PIXEL_PATTERN_DOTS_87_5, GP_PIXEL_G1_UB)
TESTCASE(d9375_ub, GP_PIXEL_PATTERN_DOTS_93_75, GP_PIXEL_G1_UB)

/*
 * Two bits per pixel, where a pattern is not only a way to fake a shade the
 * display does not have.
 *
 * G2 patterns between two of the four grays it does have. GA11 has a bit of
 * gray and a bit of alpha, so an opaque gray is a pattern over the color
 * channel with the alpha taken from the foreground throughout, a stipple is
 * the other way round, and the two together are a translucent gray -- which is
 * the thing a single pattern cannot express.
 *
 * GA11_UB is V in bit 0 and A in bit 1, so 2 is an opaque black, 3 an opaque
 * white and 0 a transparent one.
 */
TESTCASE_EX(g2_gray_db, GP_PIXEL_PATTERN_50, GP_PIXEL_PATTERN_FG,
            3, 0, GP_PIXEL_G2_DB)
TESTCASE_EX(g2_gray_ub, GP_PIXEL_PATTERN_DOTS_25, GP_PIXEL_PATTERN_FG,
            3, 0, GP_PIXEL_G2_UB)
TESTCASE_EX(ga11_gray, GP_PIXEL_PATTERN_50, GP_PIXEL_PATTERN_FG,
            2, 3, GP_PIXEL_GA11_UB)
TESTCASE_EX(ga11_stipple, GP_PIXEL_PATTERN_FG, GP_PIXEL_PATTERN_50,
            2, 0, GP_PIXEL_GA11_UB)
TESTCASE_EX(ga11_both, GP_PIXEL_PATTERN_50, GP_PIXEL_PATTERN_ASC_25,
            2, 1, GP_PIXEL_GA11_UB)

/*
 * Four bits per pixel, where two pixels fit in a byte, and GA22, where the
 * gray and the alpha are two bits each: 0xc is an opaque black, 0xf an opaque
 * white and 0x0 a transparent one.
 */
TESTCASE_EX(g4_gray_db, GP_PIXEL_PATTERN_50, GP_PIXEL_PATTERN_FG,
            15, 0, GP_PIXEL_G4_DB)
TESTCASE_EX(g4_gray_ub, GP_PIXEL_PATTERN_DOTS_25, GP_PIXEL_PATTERN_FG,
            15, 0, GP_PIXEL_G4_UB)
TESTCASE_EX(ga22_gray, GP_PIXEL_PATTERN_DSC_75, GP_PIXEL_PATTERN_FG,
            12, 15, GP_PIXEL_GA22_UB)
TESTCASE_EX(ga22_both, GP_PIXEL_PATTERN_50, GP_PIXEL_PATTERN_ASC_25,
            12, 0, GP_PIXEL_GA22_UB)

/* For test_over(): the alpha mask of the type and how many pixels survive. */
#define TESTCASE_OVER(nm, pat, apat, fg, bg, pt, amask, kept) \
	static struct testcase tc_##nm = {pat, apat, fg, bg, pt, amask, kept};

/*
 * Half the pixels are covered by a 50% tile and a quarter by an ascending
 * 25% one, out of the 8x8 area, and what is not covered keeps the white it
 * was filled with.
 *
 * The transparent one patterns a transparent black against a transparent
 * white: the two ends have to differ or gp_pixel_pattern() answers with a
 * plain color rather than a pattern, and a plain color is drawn.
 *
 * The last one is the control: G2 has no alpha channel, so there is nothing
 * to leave alone and every pixel has to be painted. Neither of its two ends
 * is the white the area was filled with, so nothing may survive.
 */
TESTCASE_OVER(over_ga11_stipple, GP_PIXEL_PATTERN_FG, GP_PIXEL_PATTERN_50,
              2, 0, GP_PIXEL_GA11_UB, 2, 32)
TESTCASE_OVER(over_ga11_clear, GP_PIXEL_PATTERN_50, GP_PIXEL_PATTERN_FG,
              0, 1, GP_PIXEL_GA11_UB, 2, 64)
TESTCASE_OVER(over_ga11_both, GP_PIXEL_PATTERN_50, GP_PIXEL_PATTERN_ASC_25,
              2, 0, GP_PIXEL_GA11_UB, 2, 48)
TESTCASE_OVER(over_g2, GP_PIXEL_PATTERN_50, GP_PIXEL_PATTERN_FG,
              1, 2, GP_PIXEL_G2_DB, 0, 0)

#define ENTRIES(suffix, label)                                             \
		{.name = "50 " label, .tst_fn = test_pattern,              \
		 .data = &tc_50_##suffix},                                \
		{.name = "ASC 25% " label, .tst_fn = test_pattern,         \
		 .data = &tc_asc25_##suffix},                             \
		{.name = "DSC 25% " label, .tst_fn = test_pattern,         \
		 .data = &tc_dsc25_##suffix},                             \
		{.name = "ASC 75% " label, .tst_fn = test_pattern,         \
		 .data = &tc_asc75_##suffix},                             \
		{.name = "DSC 75% " label, .tst_fn = test_pattern,         \
		 .data = &tc_dsc75_##suffix},                             \
		{.name = "Dots 6.25% " label, .tst_fn = test_pattern,      \
		 .data = &tc_d625_##suffix},                              \
		{.name = "Dots 12.5% " label, .tst_fn = test_pattern,      \
		 .data = &tc_d125_##suffix},                              \
		{.name = "Dots 25% " label, .tst_fn = test_pattern,        \
		 .data = &tc_d25_##suffix},                               \
		{.name = "Dots 75% " label, .tst_fn = test_pattern,        \
		 .data = &tc_d75_##suffix},                               \
		{.name = "Dots 87.5% " label, .tst_fn = test_pattern,      \
		 .data = &tc_d875_##suffix},                              \
		{.name = "Dots 93.75% " label, .tst_fn = test_pattern,     \
		 .data = &tc_d9375_##suffix},

const struct tst_suite tst_suite = {
	.suite_name = "Pattern Testsuite",
	.tests = {
		ENTRIES(db, "G1_DB")
		ENTRIES(ub, "G1_UB")

		{.name = "50 G2_DB", .tst_fn = test_pattern,
		 .data = &tc_g2_gray_db},

		{.name = "Dots 25% G2_UB", .tst_fn = test_pattern,
		 .data = &tc_g2_gray_ub},

		{.name = "Gray GA11", .tst_fn = test_pattern,
		 .data = &tc_ga11_gray},

		{.name = "Alpha stipple GA11", .tst_fn = test_pattern,
		 .data = &tc_ga11_stipple},

		{.name = "Gray and alpha GA11", .tst_fn = test_pattern,
		 .data = &tc_ga11_both},

		{.name = "Gray G4_DB", .tst_fn = test_pattern,
		 .data = &tc_g4_gray_db},

		{.name = "Gray G4_UB", .tst_fn = test_pattern,
		 .data = &tc_g4_gray_ub},

		{.name = "Gray GA22", .tst_fn = test_pattern,
		 .data = &tc_ga22_gray},

		{.name = "Gray and alpha GA22", .tst_fn = test_pattern,
		 .data = &tc_ga22_both},

		{.name = "Alpha stipple over content GA11",
		 .tst_fn = test_over, .data = &tc_over_ga11_stipple},

		{.name = "Fully transparent over content GA11",
		 .tst_fn = test_over, .data = &tc_over_ga11_clear},

		{.name = "Gray and alpha over content GA11",
		 .tst_fn = test_over, .data = &tc_over_ga11_both},

		{.name = "Pattern over content G2", .tst_fn = test_over,
		 .data = &tc_over_g2},

		{.name = NULL}
	}
};
