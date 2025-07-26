// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @file gp_icc.h
  * @brief ICC profile parser.
  */

#ifndef LOADERS_GP_ICC_H
#define LOADERS_GP_ICC_H

#include <loaders/gp_io.h>
#include <loaders/gp_data_storage.h>

/**
 * @brief Parses an ICC profile.
 *
 * This function is internally called by the JPEG and TIFF loaders.
 *
 * The ICC profile must start at the current offset in the I/O. After the
 * function returns the offset in I/O has been advanced after the ICC profile.
 *
 * The data are stored in the storage as key value pairs under the
 * "ICC Profile" key.
 *
 * Supported ICC keys are:
 *  - "Version"
 *  - "Created"
 *  - "Color Space"
 *  - "Class"
 *  - "Type"
 *  - "Primary Platform"
 *  - "Copyright"
 *  - "Description"
 *
 *  @param io A readable I/O.
 *  @param storage A storage to store the data to.
 *
 *  @return Zero on success, non-zero on a parser failure.
 */
int gp_read_icc(gp_io *io, gp_storage *storage);

#endif /* LOADERS_GP_ICC_H */
