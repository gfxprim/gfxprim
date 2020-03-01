/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

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
