// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
 */

/*

  This is a fallback implementation of clipboard for the case that backend does
  not implement it. E.g. if we are running on framebuffer there is no way how
  to propagate clipboard so we just save clipboard on copy and return it on
  paste.

 */

#include <string.h>
#include <core/gp_debug.h>
#include <backends/gp_backend.h>
#include <backends/gp_clipboard.h>

int gp_backend_clipboard(gp_backend *self, gp_clipboard *op)
{
	switch (op->op) {
	case GP_CLIPBOARD_SET:
		GP_DEBUG(4, "Setting clipboard to %s:%zu", op->str, op->len);
	break;
	case GP_CLIPBOARD_REQUEST:
		GP_DEBUG(4, "Requesting clipboard data");
	break;
	case GP_CLIPBOARD_GET:
		GP_DEBUG(4, "Getting clipboard data");
	break;
	case GP_CLIPBOARD_CLEAR:
		GP_DEBUG(4, "Clearing clipboard data");
	break;
	default:
		return 1;
	}

	if (self->clipboard)
		return self->clipboard(self, op);

	switch (op->op) {
	case GP_CLIPBOARD_SET:
		free(self->clipboard_data);
		if (op->len)
			self->clipboard_data = strndup(op->str, op->len);
		else
			self->clipboard_data = strdup(op->str);

		return 0;
	break;
	case GP_CLIPBOARD_REQUEST:
		gp_backend_clipboard_ready(self);
		return 0;
	break;
	case GP_CLIPBOARD_GET:
		if (!self->clipboard_data)
			return 1;
		op->ret = strdup(self->clipboard_data);
		return 0;
	break;
	case GP_CLIPBOARD_CLEAR:
		free(self->clipboard_data);
		self->clipboard_data = NULL;
		return 0;
	break;
	}

	return 1;
}
