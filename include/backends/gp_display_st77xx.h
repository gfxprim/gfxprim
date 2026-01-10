// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_display_st77xx.h
 * @brief Driver for st7796, st7789, st7735 displays.
 */

#ifndef GP_DISPLAY_ST77XX_H
#define GP_DISPLAY_ST77XX_H

#include <stdint.h>

/**
 * @brief Display quirks.
 */
enum gp_display_st77xx_flags {
	/**
	 * @brief Enable inverse mode.
	 *
	 * Some displays are wired so that inverse has to be enabled in order
	 * to get a normal image, some are wired that inversion has to be off.
	 */
	GP_DISPLAY_ST77XX_INV = 0x01,

	/**
	 * @brief Switch between RGB and BGR.
	 *
	 * There are displays that have inversed order.
	 */
	GP_DISPLAY_ST77XX_BGR = 0x02,

	/**
	 * @brief Mirrors X.
	 */
	GP_DISPLAY_ST77XX_MIRROR_X = 0x04,

	/**
	 * @brief Mirrors Y.
	 */
	GP_DISPLAY_ST77XX_MIRROR_Y = 0x08,
};

/**
 * @brief Initialize st77xx display driver.
 *
 * @param conn_id A connection mapping name.
 * @param w A display width.
 * @param h A display height.
 * @param x_off A display x offset, some displays does not start at 0.
 * @param y_off A display y offset, some displays does not start at 0.
 * @param dpi A display DPI.
 * @param flags Display quirks.
 */
gp_backend *gp_display_st77xx_init(const char *conn_id,
                                   uint16_t w, uint16_t h, uint16_t x_off, uint16_t y_off,
                                   unsigned int dpi, enum gp_display_st77xx_flags flags);

#endif /* GP_DISPLAY_ST77XX_H */
