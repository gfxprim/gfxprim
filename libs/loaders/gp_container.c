// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <errno.h>

#include <core/gp_debug.h>
#include <loaders/gp_container.h>

int gp_container_seek(gp_container *self, int offset,
                      enum gp_container_whence whence)
{
	if (!self->ops->seek) {
		GP_DEBUG(1, "Seek not implemented in %s container",
		         self->ops->type);
		errno = ENOSYS;
		return ENOSYS;
	}

	return self->ops->seek(self, offset, whence);
}

int gp_container_load_ex(gp_container *self, gp_pixmap **img,
                         gp_storage *storage, gp_progress_cb *callback)
{
	if (!self->ops->load_ex) {
		GP_DEBUG(1, "Load not implemented in %s container",
		         self->ops->type);
		errno = ENOSYS;
		return ENOSYS;
	}

	return self->ops->load_ex(self, img, storage, callback);
}
