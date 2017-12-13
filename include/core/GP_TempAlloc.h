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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Temporary block allocator implementation.

  Creates pool for block allocation (small ones are done on the stack, bigger
  using malloc).

  The usage is:

  // Creates new allocation pool
  gp_temp_alloc_create(buf, 3 * 1024);

  // Make use of it
  int *R = gp_temp_alloc_get(buf, 1024);
  int *G = gp_temp_alloc_get(buf, 1024);
  int *B = gp_temp_alloc_get(buf, 1024);

  ...

  // Free it
  gp_temp_allocDestroy(buf);

 */

#ifndef CORE_GP_TEMP_ALLOC_H
#define CORE_GP_TEMP_ALLOC_H

#ifdef __linux__
# include <alloca.h>
#endif
#include <stdlib.h>

#include "core/GP_Common.h"

#ifndef GP_ALLOCA_THRESHOLD
# define GP_ALLOCA_THRESHOLD 2048
#endif

struct gp_temp_alloc {
	void *buffer;
	size_t pos;
	size_t size;
};

#define GP_TEMP_ALLOC(size) ({                                        \
	((size) > GP_ALLOCA_THRESHOLD) ? malloc(size) : alloca(size); \
})

#define gp_temp_alloc_create(name, bsize)                              \
	struct gp_temp_alloc name = {.size = (bsize), .pos = 0,       \
				    .buffer = GP_TEMP_ALLOC(bsize)};

#define gp_temp_alloc_get(self, bsize) ({           \
	GP_ASSERT(self.pos + bsize <= self.size); \
	size_t _pos = self.pos;                   \
	self.pos += bsize;                        \
	(void*)(((char*)(self.buffer)) + _pos);   \
})

#define gp_temp_alloc_arr(self, type, len) \
	gp_temp_alloc_get(self, sizeof(type) * len)

#define gp_temp_alloc_free(self) do {          \
	if (self.size > GP_ALLOCA_THRESHOLD) \
		free(self.buffer);           \
} while (0)

#define gp_temp_alloc(size) GP_TEMP_ALLOC(size)

static inline void gp_temp_free(size_t size, void *ptr)
{
	if (size > GP_ALLOCA_THRESHOLD)
		free(ptr);
}

#endif /* CORE_GP_TEMP_ALLOC_H */
