// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_text.h
 * @brief A text drawing functions.
 */

#ifndef TEXT_GP_TEXT_H
#define TEXT_GP_TEXT_H

#include <stdarg.h>

#include <core/gp_types.h>
#include <core/gp_compiler.h>
#include <text/gp_text_style.h>
#include <text/gp_text_metric.h>
#include <text/gp_fonts.h>

/**
 * @brief Text flags, mostly alignment.
 *
 * The alignment is relative to a specified point.
 */
typedef enum gp_text_flags {
	/** @brief Draws the text to the left of the point. */
	GP_ALIGN_LEFT = 0x01,
	/** @brief Centers the text at the point horizontally. */
	GP_ALIGN_CENTER = 0x02,
	/** @brief Draws the text to the right of the point. */
	GP_ALIGN_RIGHT = 0x03,
	/** @brief Horizontal alignment mask. */
	GP_ALIGN_HORIZ = 0x03,
	/** @brief Apply bearing before start of the string. */
	GP_TEXT_BEARING = 0x04,
	/** @brief Draws the text above the point. */
	GP_VALIGN_ABOVE = 0x10,
	/** @brief Alias for GP_VALIGN_ABOVE. */
	GP_VALIGN_TOP = GP_VALIGN_ABOVE,
	/** @brief Centers the text vertically at the point. */
	GP_VALIGN_CENTER = 0x20,
	/** @brief Places the text baseline at the point. */
	GP_VALIGN_BASELINE = 0x30,
	/** @brief Draws the text below the point. */
	GP_VALIGN_BELOW = 0x40,
	/** @brief Alias for GP_VALIGN_BELOW. */
	GP_VALIGN_BOTTOM = GP_VALIGN_BELOW,
	/** @brief Vertical alignment mask. */
	GP_VALIGN_VERT = 0x70,
	/**
	 * @brief Use pixmap background.
	 *
	 * Mix the alpha pixels with data read from the pixmap rather than
	 * mixing them with bg_color (which is slightly slower). This has
	 * no effect for 1bpp fonts.
	 */
	GP_TEXT_NOBG = 0x80,
} gp_text_flags;

/**
 * @brief Draws a string.
 *
 * @attention Raw version that doesn't use text aligment. Use gp_text() or
 *            gp_printf() instead.
 *
 * @param pixmap A pixmap to draw the glyph into.
 * @param style A text font and style.
 * @param x An x coordinate inside the pixmap.
 * @param y An y coordinate inside the pixmap.
 * @param flags Either 0 or any combination of GP_TEXT_BEARING and GP_TEXT_NOBG
 *              and GP_VALIGN_*.
 * @param fg_color Text color.
 * @param bg_color Background color. The background color is ignored for 1bpp
 *                 font formats.
 * @param str An UTF-8 string.
 * @param max_chars A maximum number of characters to draw, can be used to
 *                  limit the number of characters to draw from str.
 *
 * @return A width of the rendered text.
 */
gp_size gp_text_raw(gp_pixmap *pixmap, const gp_text_style *style,
                    gp_coord x, gp_coord y, gp_text_flags flags,
                    gp_pixel fg_color, gp_pixel bg_color,
                    const char *str, size_t max_chars);

/**
 * @brief Draws a string.
 *
 * The string is rendered to pixmap (horizontally) with defined text style.
 * The x and y coordinates determines point defined by aligment flags.
 *
 * @param pixmap A pixmap to draw the glyph into.
 * @param style A text font and style.
 * @param x An x coordinate inside the pixmap.
 * @param y An y coordinate inside the pixmap.
 * @param flags Either 0 or any combination of GP_TEXT_BEARING and GP_TEXT_NOBG and
 *         GP_VALIGN_*
 * @param fg_color Text color.
 * @param bg_color Background color. The background color is ignored for 1bpp
 *                 font formats.
 * @param str An UTF-8 string.
 *
 * @return A width of the rendered text.
 */
gp_size gp_text(gp_pixmap *pixmap, const gp_text_style *style,
                gp_coord x, gp_coord y, gp_text_flags flags,
                gp_pixel fg_color, gp_pixel bg_color, const char *str);

/**
 * @brief Renders a single glyph.
 *
 * The x, y coordinates are the upper left corner of the glyph box. If you want
 * to render on baseline, you have to add the ascend. The if GP_TEXT_BEARING is
 * passed in the flags, bearing is applied before the glyph is rendered.
 * Usually first letter in a line is rendered without bearing and the rest of
 * them is rendered with bearing.
 *
 * @param pixmap A pixmap to draw the glyph into.
 * @param style A text font and style.
 * @param x An x coordinate inside the pixmap.
 * @param y An y coordinate inside the pixmap.
 * @param flags Either 0 or any combination of GP_TEXT_BEARING and GP_TEXT_NOBG
 *              and GP_VALIGN_*
 * @param fg_color Text color.
 * @param bg_color Background color. The background color is ignored for 1bpp
 *                 font formats.
 * @param glyph And unicode glypth to draw.
 *
 * @return Returns glyhp advance plus any style extra spacing, i.e. how much
 *         should x move for the next glyph.
 */
gp_size gp_glyph_draw(gp_pixmap *pixmap, const gp_text_style *style,
                      gp_coord x, gp_coord y, int flags,
                      gp_pixel fg_color, gp_pixel bg_color,
                      uint32_t glyph);

/**
 * @brief Draws a string.
 *
 * Aligns a string between x1 and x2 based ond horizontal aligment flags. The
 * caller must make sure that the string will fit horizontally between x1 and x2.
 *
 * @param pixmap A pixmap to draw the glyph into.
 * @param style A text font and style.
 * @param x1 An x1 coordinate inside the pixmap.
 * @param x2 An x2 coordinate inside the pixmap.
 * @param y An y coordinate inside the pixmap.
 * @param flags Either 0 or any combination of GP_TEXT_BEARING and GP_TEXT_NOBG
 *              and GP_VALIGN_*.
 * @param fg_color Text color.
 * @param bg_color Background color. The background color is ignored for 1bpp
 *                 font formats.
 * @param str An UTF-8 string.
 *
 * @return A width of the rendered text.
 */
static inline gp_size gp_text_xxy(gp_pixmap *pixmap, const gp_text_style *style,
                                  gp_coord x1, gp_coord x2, gp_coord y, gp_text_flags flags,
                                  gp_pixel fg_color, gp_pixel bg_color, const char *str)
{
	gp_coord x, len;

	if (x1 < x2) {
		x = x1;
		len = x2 - x1;
	} else {
		x = x2;
		len = x1 - x2;
	}

	int halign = flags & GP_ALIGN_HORIZ;

	flags &= ~GP_ALIGN_HORIZ;

	switch (halign) {
	case GP_ALIGN_LEFT:
		flags |= GP_ALIGN_RIGHT;
	break;
	case GP_ALIGN_CENTER:
		x += len/2;
		flags |= GP_ALIGN_CENTER;
	break;
	case GP_ALIGN_RIGHT:
		x += len;
		flags |= GP_ALIGN_LEFT;
	break;
	}

	return gp_text(pixmap, style, x, y, flags, fg_color, bg_color, str);
}

/*
 * Same as the gp_text() but the number of characters can be limited.
 */
gp_size gp_text_ext(gp_pixmap *pixmap, const gp_text_style *style,
                    gp_coord x, gp_coord y, gp_text_flags flags,
                    gp_pixel fg_color, gp_pixel bg_color,
                    const char *str, size_t max_chars);

/**
 * @brief Draws a string.
 *
 * This is the same as gp_text() only with a printf() like format and
 * parameters.
 *
 * @param pixmap A pixmap to draw the glyph into.
 * @param style A text font and style.
 * @param x An x coordinate inside the pixmap.
 * @param y An y coordinate inside the pixmap.
 * @param flags Either 0 or any combination of GP_TEXT_BEARING and GP_TEXT_NOBG and
 *         GP_VALIGN_*
 * @param fg_color Text color.
 * @param bg_color Background color. The background color is ignored for 1bpp
 *                 font formats.
 * @param fmt A printf like format.
 * @param ... A printf like parameters.
 *
 * @return A width of the rendered text.
 */
gp_size gp_print(gp_pixmap *pixmap, const gp_text_style *style,
                 gp_coord x, gp_coord y, gp_text_flags flags,
	         gp_pixel fg_color, gp_pixel bg_color, const char *fmt, ...)
                 GP_FMT_PRINTF(8, 9);

/**
 * @brief Draws a string.
 *
 * This is va_arg variant of gp_print().
 *
 * @param pixmap A pixmap to draw the glyph into.
 * @param style A text font and style.
 * @param x An x coordinate inside the pixmap.
 * @param y An y coordinate inside the pixmap.
 * @param flags Either 0 or any combination of GP_TEXT_BEARING and GP_TEXT_NOBG and
 *         GP_VALIGN_*
 * @param fg_color Text color.
 * @param bg_color Background color. The background color is ignored for 1bpp
 *                 font formats.
 * @param fmt A printf like format.
 * @param va A printf like parameters.
 *
 * @return A width of the rendered text.
 */
gp_size gp_vprint(gp_pixmap *pixmap, const gp_text_style *style,
                  gp_coord x, gp_coord y, gp_text_flags flags,
	          gp_pixel fg_color, gp_pixel bg_color,
                  const char *fmt, va_list va)
                  GP_FMT_PRINTF(8, 0);

/**
 * @brief Draws a string.
 *
 * This is the same as gp_text_xxy() only with a printf() like format and
 * parameters.
 *
 * @param pixmap A pixmap to draw the glyph into.
 * @param style A text font and style.
 * @param x1 An x1 coordinate inside the pixmap.
 * @param x2 An x2 coordinate inside the pixmap.
 * @param y An y coordinate inside the pixmap.
 * @param flags Either 0 or any combination of GP_TEXT_BEARING and GP_TEXT_NOBG
 *              and GP_VALIGN_*.
 * @param fg_color Text color.
 * @param bg_color Background color. The background color is ignored for 1bpp
 *                 font formats.
 * @param fmt A printf like format.
 * @param ... A printf like parameters.
 *
 * @return A width of the rendered text.
 */
GP_FMT_PRINTF(9, 10)
static inline gp_size gp_print_xxy(gp_pixmap *pixmap, const gp_text_style *style,
                                   gp_coord x1, gp_coord x2, gp_coord y, gp_text_flags flags,
				   gp_pixel fg_color, gp_pixel bg_color, const char *fmt, ...)
{
	gp_coord x, len;
	gp_size ret;
	va_list va;

	if (x1 < x2) {
		x = x1;
		len = x2 - x1;
	} else {
		x = x2;
		len = x1 - x2;
	}

	int halign = flags & GP_ALIGN_HORIZ;

	flags &= ~GP_ALIGN_HORIZ;

	switch (halign) {
	case GP_ALIGN_LEFT:
		flags |= GP_ALIGN_RIGHT;
	break;
	case GP_ALIGN_CENTER:
		x += len/2;
		flags |= GP_ALIGN_CENTER;
	break;
	case GP_ALIGN_RIGHT:
		x += len;
		flags |= GP_ALIGN_LEFT;
	break;
	}

	va_start(va, fmt);
	ret = gp_vprint(pixmap, style, x, y, flags, fg_color, bg_color, fmt, va);
	va_end(va);

	return ret;
}

/**
 * @brief Clears a rectangle that would be used to draw text of size pixels.
 *
 * @param pixmap A pixmap to draw the glyph into.
 * @param style A text font and style.
 * @param x An x coordinate inside the pixmap.
 * @param y An y coordinate inside the pixmap.
 *
 */
void gp_text_clear(gp_pixmap *pixmap, const gp_text_style *style,
                   gp_coord x, gp_coord y, gp_text_flags flags,
		   gp_pixel bg_color, gp_size size);

/*
 * Dtto, but with string.
 */
void gp_text_clear_str(gp_pixmap *pixmap, const gp_text_style *style,
                       gp_coord x, gp_coord y, gp_text_flags flags,
		       gp_pixel bg_color, const char *str);

#endif /* TEXT_GP_TEXT_H */
