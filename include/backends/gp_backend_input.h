// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_backend_input.h
 * @brief An backend input driver.
 *
 * Backend input driver, allows number of different input drivers to be
 * attached to a backend.
 */

#ifndef BACKENDS_GP_BACKEND_INPUT_H
#define BACKENDS_GP_BACKEND_INPUT_H

#include <backends/gp_backend.h>
#include <utils/gp_list.h>

typedef struct gp_backend_input gp_backend_input;

/**
 * An input driver.
 */
struct gp_backend_input {
	/** @brief List pointers */
	gp_dlist_head list_head;
	/**
	 * @brief A function to destroy the input driver.
	 *
	 * Closes file descriptors, frees memory, this is called by the backend on exit.
	 */
	void (*destroy)(gp_backend_input *self);
};

/**
 * @brief Destroys all input drivers added to the driver list.
 *
 * @param self A backend.
 */
void gp_backend_input_destroy(gp_backend *self);

#endif /* BACKENDS_GP_BACKEND_INPUT_H */
