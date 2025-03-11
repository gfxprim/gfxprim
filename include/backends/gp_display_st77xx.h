// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Driver for st7789 displays.
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
 * @w A display width.
 * @h A display height.
 * @dpi A display DPI.
 * @flags Additional display flags.
 */
gp_backend *gp_display_st77xx_init(uint16_t w, uint16_t h, uint16_t x_off, uint16_t y_off,
                                   unsigned int dpi, enum gp_display_st77xx_flags flags);

#endif /* GP_DISPLAY_ST77XX_H */
