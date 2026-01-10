// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025-2026 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_display_st75256.h
 * @brief Driver for st75256 displays.
 *
 * The st75256 driver can work with 1bpp and 2bpp mode with maximal display
 * size 162x256 pixels.
 */

#ifndef GP_DISPLAY_ST75256_H
#define GP_DISPLAY_ST75256_H

#include <stdint.h>

/**
 * @brief Initialize st752561 display driver.
 *
 * @param conn A connection name, pass "help" for a list and NULL for default.
 * @param w A display width.
 * @param h A display height.
 * @param contrast Initial display contrast, 9bit value, usuall values are between 85 and 360.
 * @param dpi A display DPI.
 * @param bpp Display pixel depth 1 or 2
 *
 * @return a Display backend.
 */
gp_backend *gp_display_st75256_init(const char *conn, uint16_t w, uint16_t h,
                                    uint16_t contrast, unsigned int dpi, uint8_t bpp);

#endif /* GP_DISPLAY_ST75256_H */
