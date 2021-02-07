//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_COMMON_H
#define GP_COMMON_H

#define GP_ODD_UP(x) ({    \
	typeof(x) x__ = x; \
	x__ + !(x__%2);    \
})

#define GP_EVEN_UP(x) ({    \
	typeof(x) x__ = x;  \
	x__ + (x__%2);      \
})

#endif /* GP_COMMON_H */
