// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/*

   Backend input driver, allows number of different input drivers to be
   attached to a backend.

 */

#ifndef BACKENDS_GP_BACKEND_INPUT_H__
#define BACKENDS_GP_BACKEND_INPUT_H__

#include <backends/gp_backend.h>
#include <utils/gp_list.h>

typedef struct gp_backend_input gp_backend_input;

struct gp_backend_input {
	gp_dlist_head list_head;
	void (*destroy)(gp_backend_input *self);
};

/**
 * Destroys all input drivers added to the driver list.
 */
void gp_backend_input_destroy(gp_backend *self);

#endif /* BACKENDS_GP_BACKEND_INPUT_H__ */
