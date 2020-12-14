@ include source.t

#include <core/gp_pixmap.h>
#include <gfx/gp_gfx.h>

#include "tst_test.h"

static int bench_line(gp_pixel_type type)
{
	gp_pixmap *img = gp_pixmap_alloc(800, 600, type);

	if (img == NULL) {
		tst_err("Malloc failed");
		return TST_UNTESTED;
	}

	unsigned int i;

	for (i = 0; i < 20000; i++) {
		gp_line(img, 0 + i % 100, 0 - i % 100,
		        800 - i%200, 600 + i%200, i % 0xff);
	}

	return TST_SUCCESS;
}

static int bench_circle(gp_pixel_type type)
{
	gp_pixmap *img = gp_pixmap_alloc(1000, 1000, type);

	if (!img) {
		tst_err("Malloc failed");
		return TST_UNTESTED;
	}

	unsigned int i;

	for (i = 0; i < 10000; i++)
		gp_circle(img, img->w/2, img->h/2, i % 1000, i%0xff);

	return TST_SUCCESS;
}

static int bench_circle_seg(gp_pixel_type type)
{
	gp_pixmap *img = gp_pixmap_alloc(1000, 1000, type);

	if (!img) {
		tst_err("Malloc failed");
		return TST_UNTESTED;
	}

	unsigned int i;

	for (i = 0; i < 10000; i++)
		gp_circle_seg(img, img->w/2, img->h/2, i % 1000, 0xff, i%0xff);

	return TST_SUCCESS;
}

static int bench_fill_circle(gp_pixel_type type)
{
	gp_pixmap *img = gp_pixmap_alloc(1000, 1000, type);

	if (!img) {
		tst_err("Malloc failed");
		return TST_UNTESTED;
	}

	unsigned int i;

	for (i = 0; i < 1000; i++)
		gp_fill_circle(img, img->w/2, img->h/2, i % 1000, i%0xff);

	return TST_SUCCESS;
}

@ bpps = [["1BPP", "GP_PIXEL_G1"],
@         ["2BPP", "GP_PIXEL_G2"],
@         ["4BPP", "GP_PIXEL_G4"],
@         ["8BPP", "GP_PIXEL_G8"],
@         ["16BPP", "GP_PIXEL_RGB565"],
@         ["24BPP", "GP_PIXEL_RGB888"],
@         ["32BPP", "GP_PIXEL_xRGB8888"]]
@
@ prims = ["line", "circle", "circle_seg", "fill_circle"]
@
@ def bench(prim, bpp, pixel_type):
static int bench_{{ prim }}_{{ bpp }}(void)
{
	return bench_{{ prim }}({{ pixel_type }});
}
@ end
@
@ for prim in prims:
@  for b in bpps:
{@ bench(prim, b[0], b[1]) @}

@  end
@ end

const struct tst_suite tst_suite = {
	.suite_name = "GFX benchmark",
	.tests = {
@ for prim in prims:
@  for b in bpps:
		{.name = "{{ prim }} {{ b[0] }}", .tst_fn = bench_{{ prim }}_{{ b[0] }},
	         .bench_iter = 10},
@  end

@ end

		{.name = NULL},
	}
};
