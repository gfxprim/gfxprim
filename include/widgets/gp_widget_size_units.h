//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_size_units.h
 * @brief A widget size units.
 */
#ifndef GP_WIDGET_SIZE_UNITS_H
#define GP_WIDGET_SIZE_UNITS_H

#include <widgets/gp_widget_types.h>
#include <widgets/gp_widget_render.h>

/**
 * @brief A compound size.
 *
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
	/** @brief Size in pixel. */
	uint16_t px;
	/** @brief Size in padding units. */
	uint8_t pad;
	/** @brief Size in text ascent units. */
	uint8_t asc;
};

/**
 * @brief Returns size in pixel given gp_widget_size and rendering context.
 *
 * @param size A gp_widget_size size.
 * @param ctx A render context.
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
 * @param size A widget size string e.g. "10px 2pad 1asc"
 * @param ret A gp_widget_size to store the result to.
 *
 * @return Zero on success non-zero otherwise.
 */
int gp_widget_size_units_parse(const char *size, gp_widget_size *ret);

/**
 * @brief An initializer for #gp_widget_size.
 *
 * Could be used to construct function parameters on the fly such as:
 *
 * w = gp_pixmap_create(GP_WIDGET_SIZE(100, 0, 0),
 *                      GP_WIDGET_SIZE(100, 0, 0),
 *                      callback, NULL);
 */
#define GP_WIDGET_SIZE(px_val, pad_val, asc_val) \
	((gp_widget_size){.px = px_val, .pad = pad_val, .asc = asc_val})

/**
 * @brief An default size initializer.
 *
 * Special value used in widget constructors. When size is set to this value
 * the min size is set to a default for a given widget type.
 */
#define GP_WIDGET_SIZE_DEFAULT ((gp_widget_size){0xffff, 0xff, 0xff})

/**
 * @brief A #gp_widget_size comparsion.
 *
 * @param a A #gp_widget_size.
 * @param b A #gp_widget_size.
 *
 * @return True if sizes a and b are equal.
 */
#define GP_WIDGET_SIZE_EQ(a, b) (((a).px == (b).px) && \
                                 ((a).pad == (b).pad) && \
				 ((a).asc == (b).asc))

#endif /* GP_WIDGET_SIZE_UNITS_H */
