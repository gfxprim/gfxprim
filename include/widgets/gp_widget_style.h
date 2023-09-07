//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * Common functions to get widget font and colors based on a widget state e.g. focused/disabled/etc.
 */

#ifndef WIDGETS_GP_WIDGET_STYLE_H
#define WIDGETS_GP_WIDGET_STYLE_H

static inline const gp_text_style *gp_widget_label_text_style(gp_widget *self,
                                                              const gp_widget_render_ctx *ctx)
{
	const gp_text_style *style = ctx->font;

	switch (gp_pixel_size(ctx->pixel_type)) {
	case 1:
		if (self->focused)
			style = ctx->font_bold;
	break;
	default:
	break;
	}

	return style;
}

static inline gp_pixel gp_widget_text_color(gp_widget *self,
                                            const gp_widget_render_ctx *ctx,
                                            int render_flags)
{
	gp_pixel text_color = ctx->text_color;

	switch (gp_pixel_size(ctx->pixel_type)) {
	case 1:
	break;
	default:
		if (gp_widget_is_disabled(self, render_flags))
			text_color = ctx->col_disabled;
	break;
	}

	return text_color;
}

static inline gp_pixel gp_widget_frame_color(gp_widget *self,
                                             const gp_widget_render_ctx *ctx,
                                             int render_flags)
{
	gp_pixel frame_color = ctx->text_color;

	(void) render_flags;

	switch (gp_pixel_size(ctx->pixel_type)) {
	case 1:
	break;
	default:
		if (self->focused)
			frame_color = ctx->sel_color;
	break;
	}

	return frame_color;
}

#endif /* WIDGETS_GP_WIDGET_STYLE_H */
