// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

 /*

   ICC profile parser.

  */

#ifndef LOADERS_GP_ICC_H
#define LOADERS_GP_ICC_H

#include <loaders/gp_io.h>
#include <loaders/gp_data_storage.h>

int gp_read_icc(gp_io *io, gp_storage *storage);

#endif /* LOADERS_GP_ICC_H */
