//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_COMMON_H__
#define GP_COMMON_H__

#define GP_ODD_UP(x) ({    \
	typeof(x) x__ = x; \
	x__ + !(x__%2);    \
})

#endif /* GP_COMMON_H__ */
