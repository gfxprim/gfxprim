// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_aalib.h
 * @brief An aalib backend.
 */

#ifndef BACKENDS_GP_AALIB_H
#define BACKENDS_GP_AALIB_H

#include <backends/gp_backend.h>

/**
 * @brief Initalizes an AALib backend.
 *
 * @return A newly initialized aalib backend or NULL in a case of a failure,
 *         e.g. allib support not compiled in.
 */
gp_backend *gp_aalib_init(void);

#endif /* BACKENDS_GP_AALIB_H */
