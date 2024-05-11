// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2024 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef GP_SW_CURSOR
#define GP_SW_CURSOR

#include <stdint.h>
#include <core/gp_types.h>
#include <core/gp_pixel.h>

typedef struct gp_cursor {
	/*
	 * Offset to start drawing the cursor in pixels.
	 */
	int8_t x_off, y_off;
	/*
	 * Cursor pixmap size.
	 */
	uint8_t w, h;
	/*
	 * Four pixels per byte with value:
	 *
	 * The rows have to be byte aligned.
	 *
	 * 0 - do not draw anything
	 * 1 - foreground (all pixel bits set to 1)
	 * 2 - background (all pixel bits set to 0)
	 */
	uint8_t pixmap[];
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
 *
 */
void gp_cursor_render(gp_cursor *self, gp_pixmap *dst, uint32_t x_off, uint32_t y_off);


typedef struct gp_sw_cursor {
	/* Current cursor position */
	uint32_t x;
	uint32_t y;

	uint32_t wh;

	uint32_t hidden:1;

} gp_sw_cursor;

int gp_sw_cursor_init(gp_sw_cursor *self, gp_pixel_type type);

/**
 * @brief Updates rect from a shadow buffer and renders a sw cursor into the image.
 *
 * @src A pixmap buffer with the image to be copied into the graphics memory.
 * @dst A pointer to the graphics memory, the size and pixel type must match src.
 * @x_start A start of the buffer to update.
 * @y_start A start of the buffer to update.
 * @x_end An end of the buffer to update.
 * @y_end An end of the buffer to update.
 */
void gp_sw_cursor_update_rect(gp_pixmap *src, void *dst,
		              uint32_t x_start, uint32_t y_start,
                              uint32_t x_end, uint32_t y_end);

/**
 * @brief Moves cursor to new coordinates.
 *
 * @src A pixmap buffer with the image to be copied into the graphics memory.
 * @dst A pointer to the graphics memory, the size and pixel type must match src.
 * @x A new cursor position.
 * @y A new cursor position.
 */
void gp_sw_cursor_move(gp_pixmap *src, void *dst, uint32_t x, uint32_t y);

#endif /* GP_SW_CURSOR */
