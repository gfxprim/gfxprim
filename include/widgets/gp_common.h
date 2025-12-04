//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef WIDGETS_GP_COMMON_H
#define WIDGETS_GP_COMMON_H

#define GP_ODD_UP(x) ({    \
	typeof(x) x_tmp = x; \
	x_tmp + !(x_tmp%2);    \
})

#define GP_EVEN_UP(x) ({    \
	typeof(x) x_tmp = x;  \
	x_tmp + (x_tmp%2);      \
})

#endif /* WIDGETS_GP_COMMON_H */
