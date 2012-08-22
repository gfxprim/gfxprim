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

  This code could create a buffer with either left or right barrier. Barrier is
  a page that exists right before respectively right after the buffer and is
  set to PROT_NONE (reading or writing at adresses in such page causes
  Segmentation Fault).

 */

#ifndef TST_ALLOC_BARRIERS_H
#define TST_ALLOC_BARRIERS_H

/*
 * Allocate memory with a barrier page at the right side of the buffer
 * (right == higher addresses).
 *
 * Returns NULL in case allocation or mprotect has failed.
 */
void *tst_alloc_barrier_right(size_t size);

/*
 * Free allocated buffer.
 */
void tst_free_barrier_right(void *ptr, size_t size);

/*
 * Allocate memory with barrier page at the left side of the buffer.
 *
 * Returns NULL in case allocation or mprotect has failed.
 */
void *tst_alloc_barrier_left(size_t size);

/*
 * Free allocated buffer.
 */
void tst_free_barrier_left(void *ptr, size_t size);

#endif /* TST_ALLOC_BARRIERS_H */
