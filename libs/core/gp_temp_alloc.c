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
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/gp_debug.h"
#include "core/GP_TempAlloc2.h"

void *gp_temp_alloc_(gp_temp_alloc *self, size_t size)
{
	void **ret = malloc(size + 2 * sizeof(void*));

	if (!ret)
		return NULL;

	ret[0] = NULL;
	ret[1] = self->last;

	if (!self->first) {
		self->first = ret;
		self->last = ret;
	} else {
		void** last = self->last;
		last[0] = ret;
		self->last = ret;
	}

	return &ret[2];
}

void gp_temp_alloc_save(gp_temp_alloc *self,
                        gp_temp_alloc_mark *mark)
{
	mark->stack_allocated = self->stack_allocated;
	mark->last = self->last;
}

void gp_temp_allocRestore(gp_temp_alloc *self,
                          gp_temp_alloc_mark *mark)
{
	self->stack_allocated = mark->stack_allocated;
	void **i, **j, **last = mark->last;

	if (!last)
		return;

	i = *last;

	while (i) {
		j = *i;
		free(i);
		i = j;
	}

	last[0] = NULL;
}

void gp_temp_allocDestroy(gp_temp_alloc *self)
{
	gp_temp_alloc_mark mark = {.last = self->first};

	gp_temp_allocRestore(self, &mark);
}
