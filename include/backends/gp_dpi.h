// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_dpi.h
 * @brief DPI calculation and conversions.
 */

#ifndef BACKENDS_GP_DPI_H
#define BACKENDS_GP_DPI_H

#include <core/gp_debug.h>

/**
 * @brief Calculates DPI from display physical size and resolution.
 *
 * @param w Width in pixels.
 * @param w_mm Width in mm.
 * @param h Height in pixels.
 * @param h_mm Height in mm.
 * @return An average DPI.
 */
static inline unsigned int gp_dpi_from_size(unsigned int w, unsigned int w_mm,
                                            unsigned int h, unsigned int h_mm)
{
	unsigned int dpi = ((w * 25.4) / w_mm + (h * 25.4) / h_mm + 1)/2;

	GP_DEBUG(1, "Resolution %ux%u - %ummx%umm - dpi %u", w, h, w_mm, h_mm, dpi);

	return dpi;
}

/**
 * @brief Converts DPI to dots/pixels per mm.
 *
 * @param dpi A DPI usually backend->dpi.
 * @return A PPMM a number of pixels per mm.
 */
static inline float gp_dpi_to_ppmm(unsigned int dpi)
{
	return dpi / 25.4;
}

/**
 * @brief Converts milimeters to pixels.
 *
 * @param dpi A DPI usually backend->dpi.
 * @param mm Size in milimeters.
 * @return Size in pixels rounded to whole pixels.
 */
static inline gp_size gp_dpi_mm_to_px(unsigned int dpi, float mm)
{
	return gp_dpi_to_ppmm(dpi) * mm + 0.5;
}

#endif /* BACKENDS_GP_DPI_H */
