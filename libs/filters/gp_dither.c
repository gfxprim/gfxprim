// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>
#include <string.h>
#include <filters/gp_dither.gen.h>

int gp_filter_dither(gp_dither_type type,
                     const gp_pixmap *src, gp_pixmap *dst,
                     gp_progress_cb *callback)
{
	switch (type) {
	case GP_DITHER_FLOYD_STEINBERG:
		return gp_filter_floyd_steinberg(src, dst, callback);
	case GP_DITHER_SIERRA:
		return gp_filter_sierra(src, dst, callback);
	case GP_DITHER_SIERRA_LITE:
		return gp_filter_sierra_lite(src, dst, callback);
	case GP_DITHER_HILBERT_PEANO:
		return gp_filter_hilbert_peano(src, dst, callback);
	default:
		errno = EINVAL;
		return 1;
	}
}

gp_pixmap *gp_filter_dither_alloc(gp_dither_type type,
                                  const gp_pixmap *src,
                                  gp_pixel_type pixel_type,
                                  gp_progress_cb *callback)
{
	gp_pixmap *ret = gp_pixmap_alloc(src->w, src->h, pixel_type);

	if (!ret)
		return NULL;

	if (gp_filter_dither(type, src, ret, callback)) {
		gp_pixmap_free(ret);
		return NULL;
	}

	return ret;
}

static const struct dither_names {
	const char *name;
	const char *short_name;
} dither_names[GP_DITHER_MAX] = {
	[GP_DITHER_FLOYD_STEINBERG] = {"Floyd Steinberg", "fs"},
	[GP_DITHER_SIERRA] = {"Sierra", "si"},
	[GP_DITHER_SIERRA_LITE] = {"Sierra Lite", "sl"},
	[GP_DITHER_HILBERT_PEANO] = {"Hilbert Peano", "hp"},
};

const char *gp_dither_type_name(gp_dither_type dither_type)
{
	if (dither_type >= GP_ARRAY_SIZE(dither_names))
		return "Invalid";

	return dither_names[dither_type].name;
}

gp_dither_type gp_dither_type_by_name(const char *dither_name)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(dither_names); i++) {
		if (!strcasecmp(dither_name, dither_names[i].name) ||
		    !strcasecmp(dither_name, dither_names[i].short_name))
			return i;
	}

	return GP_DITHER_MAX;
}
