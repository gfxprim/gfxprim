// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @brief A cursor rendering implementation.
 * @file gp_cursor.h
 */
#ifndef GP_CURSOR
#define GP_CURSOR

#include <stdint.h>
#include <core/gp_types.h>
#include <core/gp_pixel.h>

/** @brief A cursor pixmap. */
typedef struct gp_cursor {
	/**
	 * @brief Offset to start drawing the cursor in pixels.
	 *
	 * Defines an offset of the cursor pixmap to the cursor position.
	 */
	int8_t x_off, y_off;
	/** @brief Cursor pixmap size in pixels. */
	uint8_t w, h;
	/**
	 * @brief A cursor pixmap.
	 *
	 * Four pixels per byte with value:
	 *
	 * The rows have to be byte aligned.
	 *
	 * 0 - do not draw anything
	 * 1 - foreground (all pixel bits set to 1)
	 * 2 - background (all pixel bits set to 0)
	 */
	uint8_t *pixmap;
} gp_cursor;

/**
 * Default 32x32 pixels cursors.
 *
 * 0 - Arrow
 * 1 - Text Edit (I-beam)
 * 2 - Crosshair
 * 3 - Hand
 */
extern gp_cursor *gp_cursors_32[];

/**
 * @brief Renders a cursor into a pixmap.
 *
 * @param self A cursor.
 * @param dst A pixmap to draw the cursor into.
 * @param fg A foreground color.
 * @param bg A background color.
 * @param x_off An x offset.
 * @param y_off An y offset.
 */
void gp_cursor_render(gp_cursor *self, gp_pixmap *dst,
                      gp_pixel fg, gp_pixel bg, uint32_t x_off, uint32_t y_off);

#endif /* GP_CURSOR */
