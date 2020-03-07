// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

 /*

    Library version.

  */

#ifndef CORE_GP_VERSION_H
#define CORE_GP_VERSION_H

#define GP_VER_MAJOR 1
#define GP_VER_MINOR 0
#define GP_VER_MICRO 0

#define GP_STRX(x) GP_STR(x)
#define GP_STR(x) #x

#define GP_VER_STR GP_STRX(GP_VER_MAJOR) "." \
                   GP_STRX(GP_VER_MINOR) "." \
                   GP_STRX(GP_VER_MICRO) "-rc1"

#endif /* CORE_GP_VERSION_H */
