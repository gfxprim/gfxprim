//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_SIZE_UNITS_H
#define GP_WIDGET_SIZE_UNITS_H

#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_render.h>

/*
 * There are two fundamental constants that are used for widget size
 * computation. These are padding and text ascent. For example button widget
 * minimal height consists of ascent and two paddings one on the top and one
 * for the bottom.
 *
 * This structure describes a size as a sum of three different values these are
 * pixels, padding and ascent. The later two are relative to a render context
 * that defines the text ascent and padding.
 */
struct gp_widget_size {
	uint16_t px;
	uint8_t pad;
	uint8_t asc;
};

/**
 * @brief Returns size in pixel given gp_widget_size and rendering context.
 *
 * @size A gp_widget_size size.
 * @ctx A render context.
 *
 * @return A size in pixels.
 */
static inline unsigned int gp_widget_size_units_get(gp_widget_size *size,
                                                    const gp_widget_render_ctx *ctx)
{
	return size->px + ctx->padd * size->pad +
	       gp_text_ascent(ctx->font) * size->asc;
}

/**
 * @brief Parses widget size from a string.
 *
 * @size A widget size string e.g. "10px 2pad 1asc"
 * @ret A gp_widget_size to store the result to.
 *
 * @return Zero on success non-zero otherwise.
 */
int gp_widget_size_units_parse(const char *size, gp_widget_size *ret);

/*
 * Could be used to construct function parameters on the fly such as:
 *
 * w = gp_pixmap_create(GP_WIDGET_SIZE(100, 0, 0),
 *                      GP_WIDGET_SIZE(100, 0, 0),
 *                      callback, NULL);
 */
#define GP_WIDGET_SIZE(px_val, pad_val, asc_val) \
	((gp_widget_size){.px = px_val, .pad = pad_val, .asc = asc_val})

/*
 * Special value used in widget constructors. When size is set to this value
 * the min size is set to a default for a given widget type.
 */
#define GP_WIDGET_SIZE_DEFAULT ((gp_widget_size){0xffff, 0xff, 0xff})

#define GP_WIDGET_SIZE_EQ(a, b) (((a).px == (b).px) && \
                                 ((a).pad == (b).pad) && \
				 ((a).asc == (b).asc))

#endif /* GP_WIDGET_SIZE_UNITS_H */
