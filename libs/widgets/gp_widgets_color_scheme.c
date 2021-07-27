//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <core/gp_debug.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widgets_color_scheme.h>

#include "gp_widgets_internal.h"

struct color {
	uint8_t r, g, b;
};

struct color_scheme {
	struct color text_color;
	struct color bg_color;
	struct color fg_color;
	struct color fg2_color;
	struct color sel_color;
	struct color alert_color;
	struct color warn_color;
	struct color accept_color;
};

static struct color_scheme dark_scheme = {
	.text_color   = {0xf0, 0xf3, 0xf6},
	.bg_color     = {0x0a, 0x0c, 0x10},
	.fg_color     = {0x27, 0x2b, 0x33},
	.fg2_color    = {0x29, 0x4d, 0x75},
	.sel_color    = {0x40, 0x9e, 0xff},
	.alert_color  = {0xff, 0x55, 0x00},
	.accept_color = {0x00, 0xbb, 0x00},
	.warn_color   = {0xfc, 0xd1, 0x2a},
};

static struct color_scheme light_scheme = {
	.text_color   = {0x00, 0x00, 0x00},
	.bg_color     = {0xdd, 0xdd, 0xdd},
	.fg_color     = {0xee, 0xee, 0xee},
	.fg2_color    = {0x77, 0xbb, 0xff},
	.sel_color    = {0x11, 0x99, 0xff},
	.alert_color  = {0xff, 0x55, 0x00},
	.accept_color = {0x00, 0xbb, 0x00},
	.warn_color   = {0xfc, 0xd1, 0x2a},
};

#define RGB_TO_COLOR(ctx, scheme, color_name) \
	(ctx).color_name = gp_rgb_to_pixmap_pixel((scheme)->color_name.r, \
	                                          (scheme)->color_name.g, \
	                                          (scheme)->color_name.b, (ctx).buf)

void __attribute__((visibility ("hidden"))) widgets_color_scheme_load(void)
{
	struct color_scheme *scheme = NULL;

	if (ctx.color_scheme == GP_WIDGET_COLOR_SCHEME_DEFAULT)
		ctx.color_scheme = GP_WIDGET_COLOR_SCHEME_LIGHT;

	switch (ctx.color_scheme) {
	case GP_WIDGET_COLOR_SCHEME_LIGHT:
		scheme = &light_scheme;
	break;
	case GP_WIDGET_COLOR_SCHEME_DARK:
		scheme = &dark_scheme;
	break;
	default:
		GP_WARN("Invalid ctx->color_scheme id %i", ctx.color_scheme);
		return;
	}

	RGB_TO_COLOR(ctx, scheme, text_color);
	RGB_TO_COLOR(ctx, scheme, bg_color);
	RGB_TO_COLOR(ctx, scheme, fg_color);
	RGB_TO_COLOR(ctx, scheme, fg2_color);
	RGB_TO_COLOR(ctx, scheme, sel_color);
	RGB_TO_COLOR(ctx, scheme, alert_color);
	RGB_TO_COLOR(ctx, scheme, accept_color);
	RGB_TO_COLOR(ctx, scheme, warn_color);
}

void gp_widgets_color_scheme_toggle(void)
{
	if (ctx.color_scheme == GP_WIDGET_COLOR_SCHEME_LIGHT)
		ctx.color_scheme = GP_WIDGET_COLOR_SCHEME_DARK;
	else
		ctx.color_scheme = GP_WIDGET_COLOR_SCHEME_LIGHT;

	widgets_color_scheme_load();
	widget_render_refresh();
}

void gp_widgets_color_scheme_set(enum gp_widgets_color_scheme color_scheme)
{
	switch (color_scheme) {
	case GP_WIDGET_COLOR_SCHEME_DEFAULT:
		ctx.color_scheme = GP_WIDGET_COLOR_SCHEME_LIGHT;
	break;
	case GP_WIDGET_COLOR_SCHEME_LIGHT:
	case GP_WIDGET_COLOR_SCHEME_DARK:
		ctx.color_scheme = color_scheme;
	break;
	default:
		GP_WARN("Invalid color scheme id %i", color_scheme);
		return;
	}

	widgets_color_scheme_load();
	widget_render_refresh();
}

enum gp_widgets_color_scheme gp_widgets_color_scheme_get(void)
{
	return ctx.color_scheme;
}
