// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_DPI_H
#define BACKENDS_GP_DPI_H

#include <core/gp_debug.h>

/**
 * Calculates DPI from display physical size and resolution.
 *
 * @w Width in pixels.
 * @w_mm Width in mm.
 * @h Height in pixels.
 * @h_mm Height in mm.
 * @return DPI
 */
static inline unsigned int gp_dpi_from_size(unsigned int w, unsigned int w_mm,
                                            unsigned int h, unsigned int h_mm)
{
	unsigned int dpi = ((w * 25.4) / w_mm + (h * 25.4) / h_mm + 1)/2;

	GP_DEBUG(1, "Resolution %ux%u - %ummx%umm - dpi %u", w, h, w_mm, h_mm, dpi);

	return dpi;
}

/**
 * Converts DPI to dots/pixels per mm.
 *
 * @dpi A DPI usually backend->dpi.
 * @return A PPMM a number of pixels per mm.
 */
static inline float gp_dpi_to_ppmm(unsigned int dpi)
{
	return dpi / 25.4;
}

/**
 * Converts milimeters to pixels.
 *
 * @mm Size in milimeters.
 * @return Size in pixels.
 */
static inline gp_size gp_dpi_mm_to_px(unsigned int dpi, float mm)
{
	return gp_dpi_to_ppmm(dpi) * mm + 0.5;
}

#endif /* BACKENDS_GP_DPI_H */
