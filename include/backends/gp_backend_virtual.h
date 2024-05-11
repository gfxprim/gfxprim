// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_backend_virtual.h
 * @brief A virtual backend.
 *
 * Virtual backend, could emulate backed with any pixel type on the top of an
 * initalized backend, e.g. you can emulate 4bpp grayscale backend on the top
 * of RBG888 X11.
 *
 * Mostly useful for testing.
 */

#ifndef BACKENDS_GP_BACKEND_VIRTUAL_H
#define BACKENDS_GP_BACKEND_VIRTUAL_H

#include <core/gp_pixel.h>
#include <backends/gp_backend.h>

/**
 * @brief A virtual backend flags.
 */
enum gp_backend_virt_flags {
	/** @brief If set virtual backend exit calls 'parent' exit as well */
	GP_BACKEND_CALL_EXIT = 0x01,
};

/**
 * @brief Creates an virtual backend on the top of the existing backend.
 *
 * @param backend Already initialized backend.
 * @param pixel_type A pixel type to emulate.
 * @param flags A gp_backend_virt_flags.
 *
 * @return A newly initialized virtual backend or NULL in a case of a failure.
 */
gp_backend *gp_backend_virt_init(gp_backend *backend,
                                 gp_pixel_type pixel_type,
				 enum gp_backend_virt_flags flags);

#endif /* BACKENDS_GP_BACKEND_VIRTUAL_H */
