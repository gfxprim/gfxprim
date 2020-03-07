// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  This code could create a buffer with either left or right canary. Canary is
  a page that exists right before respectively right after the buffer and is
  set to PROT_NONE (reading or writing at adresses in such page causes
  Segmentation Fault).

 */

#ifndef TST_ALLOC_CANARY_H
#define TST_ALLOC_CANARY_H

/*
 * Allocate memory with a canary page at the right side of the buffer
 * (right == higher addresses).
 *
 * Returns NULL in case allocation or mprotect has failed.
 */
void *tst_malloc_canary_right(size_t size);

/*
 * Free allocated buffer.
 */
void tst_free_canary_right(void *ptr, size_t size);

/*
 * Allocate memory with canary page at the left side of the buffer.
 *
 * Returns NULL in case allocation or mprotect has failed.
 */
void *tst_malloc_canary_left(size_t size);

/*
 * Free allocated buffer.
 */
void tst_free_canary_left(void *ptr, size_t size);

#endif /* TST_ALLOC_CANARY_H */
