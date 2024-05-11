// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_backend_init.h
 * @brief General backend init
 *
 * Simplified backend initalization interface good enough for most of the cases.
 */

#ifndef BACKENDS_GP_BACKEND_INIT_H
#define BACKENDS_GP_BACKEND_INIT_H

#include <backends/gp_backend.h>

/**
 * @brief Generic backend initialization.
 *
 * Example usage:
 * @code
 *	const char *backend_opts = NULL;
 *	gp_backend *backend;
 *
 *	...
 *
 *	while ((opt = getopt(argc, argv, "b:")) {
 *		switch (opt) {
 *		case 'b':
 *			backend_opts = optarg;
 *		break;
 *		...
 *		}
 *	}
 *
 *	backend = gp_backend_init(backend_opts, 0, 0, "Backend init example");
 *	if (!backend)
 *		exit(1);
 *
 *	...
 * @endcode
 *
 * @param params A backend init string, in most cases passed down by an user,
 *               when NULL is passed in params the library attempts to
 *               autodetect which backend to use. The params format is
 *               "backend_name:backend_params" e.g. "SDL:fs" is string for
 *               fullscreen SDL backend. If "help" is passed as a backend name
 *               a help is printed into the stderr.
 * @param pref_w Preferred width pass 0 for default.
 * @param pref_h Preferred height pass 0 for default.
 * @param caption A caption used in a case that backends runs in a window with a caption.
 * @return Initalized backend or NULL in case of failure.
 */
gp_backend *gp_backend_init(const char *params,
                            gp_size pref_w, gp_size pref_h,
                            const char *caption);

/**
 * @brief Prints information about all backends and parameters.
 *
 * Prints information about all backends including description of their
 * parameters into the stderr.
 */
static inline void gp_backend_init_help(void)
{
	gp_backend_init("help", 0, 0, NULL);
}

#endif /* BACKENDS_GP_BACKEND_INIT_H */
