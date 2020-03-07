// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef CORE_GP_BYTE_ORDER_H
#define CORE_GP_BYTE_ORDER_H

/*
 * To get macros as __BYTE_ORDER __BIG_ENDIAN and __LITTLE_ENDIAN
 */
#ifdef __linux__
# include <endian.h>
#else /* BSD Family */
# include <machine/endian.h>
# define __BYTE_ORDER _BYTE_ORDER
# define __BIG_ENDIAN _BIG_ENDIAN
# define __LITTLE_ENDIAN _LITTLE_ENDIAN
#endif

#endif /* CORE_GP_BYTE_ORDER_H */
