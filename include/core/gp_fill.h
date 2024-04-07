// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_fill.h
 * @brief Fills a pixmap.
 */

#ifndef CORE_GP_FILL_H
#define CORE_GP_FILL_H

#include <core/gp_types.h>

/**
 * @brief Fills pixmap with given pixel value.
 * @ingroup gfx
 *
 * @param pixmap A pixmap to be filled.
 * @param val A pixel value to fill the pixmap with.
 */
void gp_fill(gp_pixmap *pixmap, gp_pixel val);

#endif /* CORE_GP_FILL_H */
