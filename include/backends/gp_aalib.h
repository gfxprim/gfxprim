// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_AALIB_H
#define BACKENDS_GP_AALIB_H

#include <backends/gp_backend.h>

/*
 * Initalize AALib.
 *
 * TODO: add a way to pass aalib params
 */
gp_backend *gp_aalib_init(void);

#endif /* BACKENDS_GP_AALIB_H */
