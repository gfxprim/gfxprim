// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

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
