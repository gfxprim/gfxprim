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

#include "core/GP_Debug.h"
#include "loaders/GP_Container.h"

int GP_ContainerSeek(GP_Container *self, int offset,
                     enum GP_ContainerWhence whence)
{
	if (!self->ops->Seek) {
		GP_DEBUG(1, "Seek not implemented in %s container",
		         self->ops->type);
		errno = ENOSYS;
		return ENOSYS;
	}

	return self->ops->Seek(self, offset, whence);
}

int GP_ContainerLoadEx(GP_Container *self, GP_Context **img,
                       GP_DataStorage *storage, GP_ProgressCallback *callback)
{
	if (!self->ops->LoadEx) {
		GP_DEBUG(1, "Load not implemented in %s container",
		         self->ops->type);
		errno = ENOSYS;
		return ENOSYS;
	}

	return self->ops->LoadEx(self, img, storage, callback);
}
