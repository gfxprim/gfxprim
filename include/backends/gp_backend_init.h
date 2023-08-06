// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Simplified backend initalization interface good enough for most of the cases.

 */

#ifndef BACKENDS_GP_BACKEND_INIT_H
#define BACKENDS_GP_BACKEND_INIT_H

#include <backends/gp_backend.h>

/**
 * @brief Initalize backend by a string.
 *
 * The params format is:
 *
 * "backend_name:backend_params"
 *
 * For example "SDL:fs" is string for fullscreen SDL backend.
 *
 * @params A backend init string, in most cases passed down by an user.
 *         If "help" is passed as a backend name a help is printed into the stderr.
 * @pref_w Preferred width.
 * @pref_h Preferred height.
 * @caption A caption used in a case that backends runs in a window with a caption.
 * @return Initalized backend or NULL in case of failure.
 */
gp_backend *gp_backend_init(const char *params,
                            gp_size pref_w, gp_size pref_h,
                            const char *caption);

static inline void gp_backend_init_help(void)
{
	gp_backend_init("help", 0, 0, NULL);
}

#endif /* BACKENDS_GP_BACKEND_INIT_H */
