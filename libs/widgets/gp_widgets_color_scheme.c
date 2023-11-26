//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <core/gp_debug.h>
#include <widgets/gp_widget_render.h>
#include <widgets/gp_widgets_color_scheme.h>
#include <widgets/gp_widget.h>
#include <widgets/gp_widget_stock.h>

#include "gp_widgets_internal.h"

static gp_widget *theme_switch;

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
	struct color fill_color;
	struct color disabled_color;
};

static struct color_scheme dark_scheme = {
	.text_color   = {0xf0, 0xf3, 0xf6},
	.fg_color     = {0x00, 0x00, 0x00},
	.bg_color     = {0x0d, 0x10, 0x1a},
	.fg2_color    = {0x10, 0x1a, 0x50},
	.sel_color    = {0x20, 0x50, 0x88},
	.alert_color  = {0xbb, 0x30, 0x00},
	.accept_color = {0x00, 0xbb, 0x00},
	.warn_color   = {0x90, 0x80, 0x00},
	.fill_color   = {0x00, 0x00, 0x00},
	.disabled_color = {0xaa, 0xaa, 0xaa},
};

static struct color_scheme light_scheme = {
	.text_color   = {0x00, 0x00, 0x00},
	.bg_color     = {0xdd, 0xdd, 0xdd},
	.fg_color     = {0xee, 0xee, 0xee},
	.fg2_color    = {0x66, 0xaa, 0xee},
	.sel_color    = {0x11, 0x99, 0xff},
	.alert_color  = {0xff, 0x22, 0x00},
	.accept_color = {0x00, 0xbb, 0x00},
	.warn_color   = {0xdc, 0xb1, 0x0a},
	.fill_color   = {0x44, 0x44, 0x44},
	.disabled_color = {0x66, 0x66, 0x66},
};

static const char *color_names[] = {
	/* theme colors */
	[GP_WIDGETS_COL_TEXT] = "text",
	[GP_WIDGETS_COL_BG] = "bg",
	[GP_WIDGETS_COL_FG] = "fg",
	[GP_WIDGETS_COL_HIGHLIGHT] = "highlight",
	[GP_WIDGETS_COL_ALERT] = "alert",
	[GP_WIDGETS_COL_WARN] = "warn",
	[GP_WIDGETS_COL_ACCEPT] = "accept",
	/* 16 color palette */
	[GP_WIDGETS_COL_BLACK] = "black",
	[GP_WIDGETS_COL_RED] = "red",
	[GP_WIDGETS_COL_GREEN] = "green",
	[GP_WIDGETS_COL_YELLOW] = "yellow",
	[GP_WIDGETS_COL_BLUE] = "blue",
	[GP_WIDGETS_COL_MAGENTA] = "magenta",
	[GP_WIDGETS_COL_CYAN] = "cyan",
	[GP_WIDGETS_COL_GRAY] = "light gray",
	[GP_WIDGETS_COL_BR_BLACK] = "dark gray",
	[GP_WIDGETS_COL_BR_RED] = "bright red",
	[GP_WIDGETS_COL_BR_GREEN] = "bright green",
	[GP_WIDGETS_COL_BR_YELLOW] = "bright yellow",
	[GP_WIDGETS_COL_BR_BLUE] = "bright blue",
	[GP_WIDGETS_COL_BR_MAGENTA] = "bright magenta",
	[GP_WIDGETS_COL_BR_CYAN] = "bright cyan",
	[GP_WIDGETS_COL_WHITE] = "white",
};

#define RGB_TO_PIXEL(ctx, scheme, color_name, color_index) \
	ctx.colors[color_index] = gp_rgb_to_pixmap_pixel((scheme)->color_name.r, \
	                                                  (scheme)->color_name.g, \
	                                                  (scheme)->color_name.b, (ctx).buf)

#define RGB_TO_PIXEL2(ctx, color_index, r, g, b) \
	ctx.colors[color_index] = gp_rgb_to_pixmap_pixel(r, g, b, (ctx).buf)

enum gp_widgets_color gp_widgets_color_name_idx(const char *name)
{
	size_t i;

	for (i = 0; i < GP_ARRAY_SIZE(color_names); i++) {
		if (!color_names[i])
			continue;

		if (!strcmp(color_names[i], name))
			return i;
	}

	return -1;
}

gp_pixel gp_widgets_color(const gp_widget_render_ctx *ctx, enum gp_widgets_color color)
{
	if (color < 0 || color >= GP_WIDGETS_COL_CNT) {
		GP_WARN("Invalid color index %i\n", color);
		return 0;
	}

	return ctx->colors[color];
}

static void widgets_color_scheme_load_1bpp(void)
{
	gp_pixel fg, bg;

	switch (ctx.color_scheme) {
	case GP_WIDGET_COLOR_SCHEME_LIGHT:
		fg = 0;
		bg = 1;
	break;
	case GP_WIDGET_COLOR_SCHEME_DARK:
		fg = 1;
		bg = 0;
	break;
	case GP_WIDGET_COLOR_SCHEME_DEFAULT:
	break;
	}

	ctx.text_color = fg;
	ctx.alert_color = fg;
	ctx.warn_color = fg;
	ctx.accept_color = fg;
	ctx.col_disabled = fg;
	ctx.hl_color = fg;
	ctx.sel_color = fg;

	ctx.bg_color = bg;
	ctx.fg_color = bg;
}

static void widgets_color_scheme_load_rgb(struct color_scheme *scheme)
{
	RGB_TO_PIXEL(ctx, scheme, text_color, GP_WIDGETS_COL_TEXT);
	RGB_TO_PIXEL(ctx, scheme, bg_color, GP_WIDGETS_COL_BG);
	RGB_TO_PIXEL(ctx, scheme, fg_color, GP_WIDGETS_COL_FG);
	RGB_TO_PIXEL(ctx, scheme, fg2_color, GP_WIDGETS_COL_HIGHLIGHT);
	RGB_TO_PIXEL(ctx, scheme, sel_color, GP_WIDGETS_COL_SELECT);
	RGB_TO_PIXEL(ctx, scheme, alert_color, GP_WIDGETS_COL_ALERT);
	RGB_TO_PIXEL(ctx, scheme, warn_color, GP_WIDGETS_COL_WARN);
	RGB_TO_PIXEL(ctx, scheme, accept_color, GP_WIDGETS_COL_ACCEPT);
	RGB_TO_PIXEL(ctx, scheme, disabled_color, GP_WIDGETS_COL_DISABLED);
}

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

	switch (gp_pixel_size(ctx.pixel_type)) {
	case 1:
		widgets_color_scheme_load_1bpp();
	break;
	default:
		widgets_color_scheme_load_rgb(scheme);
	break;
	}

	if (gp_pixel_size(ctx.pixel_type) <= 2)
		ctx.focused_is_bold = 1;

	/* 16 colors */
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BLACK, 0x00, 0x00, 0x00);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_RED, 0xcd, 0x00, 0x00);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_GREEN, 0x00, 0xcd, 0x00);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_YELLOW, 0xcd, 0xcd, 0x00);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BLUE, 0x00, 0x00, 0xee);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_MAGENTA, 0xcd, 0x00, 0xcd);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_CYAN, 0x00, 0xcd, 0xcd);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_GRAY, 0xe5, 0xe5, 0xe5);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BR_BLACK, 0x7f, 0x7f, 0x7f);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BR_RED, 0xff, 0x00, 0x00);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BR_GREEN, 0x00, 0xff, 0x00);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BR_YELLOW, 0xff, 0xff, 0x00);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BR_BLUE, 0x5c, 0x5c, 0xff);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BR_MAGENTA, 0xff, 0x00, 0xff);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_BR_CYAN, 0x00, 0xff, 0xff);
	RGB_TO_PIXEL2(ctx, GP_WIDGETS_COL_WHITE, 0xff, 0xff, 0xff);
}

static enum gp_widget_stock_type scheme_to_type(void)
{
	switch (ctx.color_scheme) {
	case GP_WIDGET_COLOR_SCHEME_DEFAULT:
	case GP_WIDGET_COLOR_SCHEME_LIGHT:
		return GP_WIDGET_STOCK_DAY;
	break;
	case GP_WIDGET_COLOR_SCHEME_DARK:
		return GP_WIDGET_STOCK_NIGHT;
	break;
	default:
		return GP_WIDGET_STOCK_ERR;
	}
}

static void theme_switch_update(void)
{
	if (!theme_switch)
		return;

	gp_widget_stock_type_set(theme_switch, scheme_to_type());
}

void gp_widgets_color_scheme_toggle(void)
{
	if (ctx.color_scheme == GP_WIDGET_COLOR_SCHEME_LIGHT)
		ctx.color_scheme = GP_WIDGET_COLOR_SCHEME_DARK;
	else
		ctx.color_scheme = GP_WIDGET_COLOR_SCHEME_LIGHT;

	theme_switch_update();
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

static int theme_switch_on_event(gp_widget_event *ev)
{
	switch (ev->type) {
	case GP_WIDGET_EVENT_WIDGET:
		gp_widgets_color_scheme_toggle();
	break;
	case GP_WIDGET_EVENT_FREE:
		theme_switch = NULL;
	break;
	}

	return 0;
}

gp_widget *gp_widget_color_scheme_switch(void)
{
	gp_widget *ret;

	if (theme_switch)
		return NULL;

	ret = gp_widget_stock_new(scheme_to_type(), GP_WIDGET_SIZE_DEFAULT);
	if (!ret)
		return NULL;

	gp_widget_on_event_set(ret, theme_switch_on_event, NULL);

	theme_switch = ret;

	return ret;
}
