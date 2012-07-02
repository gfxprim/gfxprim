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
  GP_TempAllocCrate(buf, 3 * 1024);

  // Make use of it
  int *R = GP_TempAllocGet(buf, 1024);
  int *G = GP_TempAllocGet(buf, 1024);
  int *B = GP_TempAllocGet(buf, 1024);

  ...

  // Free it
  GP_TempAllocDestroy(buf);

 */

#ifndef CORE_GP_TEMP_ALLOC_H
#define CORE_GP_TEMP_ALLOC_H

#include <alloca.h>
#include <stdlib.h>

#include "core/GP_Common.h"

#ifndef GP_ALLOCA_THRESHOLD
# define GP_ALLOCA_THRESHOLD 2048
#endif

struct GP_TempAlloc {
	void *buffer;
	size_t pos;
	size_t size;
};

#define GP_TEMP_ALLOC(size) ({                                        \
	((size) > GP_ALLOCA_THRESHOLD) ? malloc(size) : alloca(size); \
})

#define GP_TempAllocCreate(name, bsize)                              \
	struct GP_TempAlloc name = {.size = (bsize), .pos = 0,       \
				    .buffer = GP_TEMP_ALLOC(bsize)};

#define GP_TempAllocGet(self, bsize) ({           \
	GP_ASSERT(self.pos + bsize <= self.size); \
	size_t _pos = self.pos;                   \
	self.pos += bsize;                        \
	(void*)(((char*)(self.buffer)) + _pos);   \
})

#define GP_TempAllocFree(self) do {          \
	if (self.size > GP_ALLOCA_THRESHOLD) \
		free(self.buffer);           \
} while (0)

#endif /* CORE_GP_TEMP_ALLOC_H */
