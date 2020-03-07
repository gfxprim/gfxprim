// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Virtual backend, could emulate backed with any pixel type on the top of
  initalized backend. Useful for testing.

 */

#ifndef BACKENDS_GP_BACKEND_VIRTUAL_H
#define BACKENDS_GP_BACKEND_VIRTUAL_H

#include <core/gp_pixel.h>
#include <backends/gp_backend.h>

enum gp_backend_virt_flags {
	/* If set virtual backend exit calls 'parent' exit as well */
	GP_BACKEND_CALL_EXIT = 0x01,
};

/*
 * Create an virtual backend on the top of the existing backend.
 */
gp_backend *gp_backend_virt_init(gp_backend *backend,
                                 gp_pixel_type pixel_type,
				 enum gp_backend_virt_flags flags);

#endif /* BACKENDS_GP_BACKEND_VIRTUAL_H */
