// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */
/*

  Clipboard is, by definition, asynchronous. Getting a clipboard data requires
  roundtrip to a server in most cases.

  Getting the clipboard starts by sending a clipboard request. Once clipboard
  data are ready the application will get an GP_EV_SYS_CLIPBOARD event and the
  data can be retrieved by calling the clipboard get function.

 */

#ifndef BACKENDS_GP_CLIPBOARD_H
#define BACKENDS_GP_CLIPBOARD_H

#include <backends/gp_types.h>

enum gp_clipboard_op {
	GP_CLIPBOARD_SET,
	GP_CLIPBOARD_REQUEST,
	GP_CLIPBOARD_GET,
	GP_CLIPBOARD_CLEAR,
};

struct gp_clipboard {
	enum gp_clipboard_op op;
	union {
		const char *str;
		char *ret;
	};
	size_t len;
};

/**
 * @brief An internal handler that implements clipboard operations.
 *
 * Use the static inline functions instead!
 *
 * @self A backend.
 * @op Properly filled in gp_clipboard structure.
 * @return Zero on success non-zero otherwise.
 */
int gp_backend_clipboard(gp_backend *self, gp_clipboard *op);

/**
 * @brief Sets the clipboard.
 *
 * @self A backend.
 * @str An string buffer to copy the clipboard data from.
 * @len Optional lenght to limit the string lenght. If set to zero whole string is used.
 */
static inline int gp_backend_clipboard_set(gp_backend *self, const char *str, size_t len)
{
	struct gp_clipboard op = {
		.op = GP_CLIPBOARD_SET,
		.str = str,
		.len = len,
	};

	return gp_backend_clipboard(self, &op);
}

/**
 * @brief Requests clipboard data to be retrieved.
 *
 * @self A backend.
 */
static inline int gp_backend_clipboard_request(gp_backend *self)
{
	struct gp_clipboard op = {.op = GP_CLIPBOARD_REQUEST};

	return gp_backend_clipboard(self, &op);
}

/**
 * @brief Returns clipboard data.
 *
 * The data has to be request by the gp_backend_clipboard_request() first and
 * are ready only after clipboard event arrives to the backend event queue.
 *
 * @self A backend.
 *
 * @return A clipboard data in a buffer allocated by malloc() or a NULL in a
 * case of a failure.
 */
static inline char *gp_backend_clipboard_get(gp_backend *self)
{
	struct gp_clipboard op = {.op = GP_CLIPBOARD_GET};

	gp_backend_clipboard(self, &op);

	return op.ret;
}

/**
 * @brief Pushes clipboard data ready event into a backend input queue.
 *
 * @self A backend.
 */
static inline void gp_backend_clipboard_ready(gp_backend *self)
{
	gp_ev_queue_push(self->event_queue, GP_EV_SYS, GP_EV_SYS_CLIPBOARD, 0, 0);
}

#endif /* BACKENDS_GP_CLIPBOARD_H */
