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
  
   Code to store test messages into a data structure.

  */

#ifndef TST_MSG_H
#define TST_MSG_H

struct tst_msg {
	struct tst_msg *next;
	int type;
	char msg[];
};

struct tst_msg_store {
	struct tst_msg *first;
	struct tst_msg *last;
};

/*
 * Initalize message store.
 */
static inline void tst_msg_init(struct tst_msg_store *self)
{
	self->first = NULL;
	self->last = NULL;
}

/*
 * Cleans msg store, frees memory.
 */
void tst_msg_clear(struct tst_msg_store *self);

/*
 * Appends test message to the store.
 */
int tst_msg_append(struct tst_msg_store *self, int type, const char *msg);

/*
 * Prints messages in the store.
 */
void tst_msg_print(const struct tst_msg_store *self);

#endif /* TST_MSG_H */
