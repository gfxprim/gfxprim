// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_sdl.h
 * @brief A SDL backend.
 *
 * SDL doesn't expose file descriptors, so the backend does polling.
 *
 * The backend is thread safe (the critical parts are guarded with a mutex).
 *
 * The backend is singleton, you can't have two SDL backends running at the
 * same time.
 */
#ifndef BACKENDS_GP_SDL_H
#define BACKENDS_GP_SDL_H

#include <stdint.h>
#include <backends/gp_backend.h>

/** @brief SDL backend init flags. */
enum gp_sdl_flags {
	/** @brief Start as a full screen. */
	GP_SDL_FULLSCREEN = 0x01,
	/** @brief Allow for the backend to be resizable. */
	GP_SDL_RESIZABLE  = 0x02,
};

/**
 * @brief Initalizes SDL as drawing backend.
 *
 * When backend is allready initalized, this function ignores it's parameters
 * and returns pointer to allready initalized SDL backend.
 *
 * The parameters w h and bpp are directly passed to SDL_SetVideoMode().
 *
 * If w, h and/or bpp are set to zero, SDL tries to do best fit.
 *
 * @param w A requested window width.
 * @param h A requested window height.
 * @param bpp A requested depth, bits per pixel.
 * @param flags A SDL backend init flags.
 * @param caption A window caption.
 * @return An initialized SDL backend or NULL on a failure, or if SDL wasn't
 *         compiled in.
 */
gp_backend *gp_sdl_init(gp_size w, gp_size h, uint8_t bpp,
                        enum gp_sdl_flags flags, const char *caption);

#endif /* BACKENDS_GP_SDL_H */
