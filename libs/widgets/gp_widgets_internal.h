//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGETS_INTERNAL_H
#define GP_WIDGETS_INTERNAL_H

/*
 * The application may only access this variable by a call to
 * gp_widget_render_ctx_get() that returns a const pointer that cannot be
 * modified. The ctx variable is declared with a hidden visibility so it's not
 * exported to the librray.
 */
extern struct gp_widget_render_ctx ctx;

/*
 * Repaints the application after a change of a color scheme.
 */
void widget_render_refresh(void);

/*
 * Sets the ctx->*_color variables accordingly to the color scheme.
 */
void widgets_color_scheme_load(void);

#endif /* GP_WIDGETS_INTERNAL_H */
