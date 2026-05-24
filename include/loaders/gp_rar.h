// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @brief A rar container using libarchive.
  */

#ifndef LOADERS_GP_RAR_H
#define LOADERS_GP_RAR_H

#include <loaders/gp_container.h>

extern const gp_container_ops gp_rar_ops;

/**
 * @brief buf A buffer with a first 32 bytes of a file.
 *
 * @return Non-zero if a rar signature was found.
 */
int gp_match_rar(const void *buf);

#endif /* LOADERS_GP_RAR_H */
