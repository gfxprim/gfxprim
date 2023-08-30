//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2021 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef UTILS_GP_TYPES_H
#define UTILS_GP_TYPES_H

typedef struct gp_htable gp_htable;
typedef struct gp_heap_head gp_heap_head;
typedef struct gp_fd gp_fd;

struct gp_heap_head {
	gp_heap_head *left;
	gp_heap_head *right;
	gp_heap_head *up;
	unsigned long children;
};

#endif /* UTILS_GP_TYPES_H */
