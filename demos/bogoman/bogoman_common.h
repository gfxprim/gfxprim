// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef __BOGOMAN_COMMON_H__
#define __BOGOMAN_COMMON_H__

#define SWAP(a, b) do {    \
	typeof(a) tmp = b; \
	b = a;             \
	a = tmp;           \
} while (0)

#define SIGN(a) ({                          \
	typeof(a) tmp = a;                  \
	(tmp < 0) ? -1 : (tmp > 0) ? 1 : 0; \
})

#endif /* __BOGOMAN_COMMON_H__ */
