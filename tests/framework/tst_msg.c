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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tst_test.h"

#include "tst_msg.h"

void tst_msg_clear(struct tst_msg_store *self)
{
	struct tst_msg *msg, *prev = NULL;

	for (msg = self->first; msg != NULL; msg = msg->next) {
		free(prev);
		prev = msg;
	}

	free(prev);

	self->first = NULL;
	self->last = NULL;
}

int tst_msg_append(struct tst_msg_store *self, int type, const char *msg_text)
{
	size_t len = strlen(msg_text);
	struct tst_msg *msg;

	msg = malloc(sizeof(struct tst_msg) + len + 1);

	if (msg == NULL) {
		tst_warn("tst_msg: malloc() failed: %s", strerror(errno));
		return 1;
	}

	msg->type = type;
	msg->next = NULL;
	strcpy(msg->msg, msg_text);

	if (self->last == NULL) {
		self->first = msg;
		self->last = msg;
	} else {
		self->last->next = msg;
		self->last = msg;
	}

	return 0;
}

static char type_to_char(enum tst_report_type type)
{
	switch (type) {
	case TST_MSG:
		return 'M';
	case TST_WARN:
		return 'W';
	case TST_ERR:
		return 'E';
	default:
		return '?';
	}
}

void tst_msg_print(const struct tst_msg_store *self)
{
	struct tst_msg *msg;

	for (msg = self->first; msg != NULL; msg = msg->next)
		fprintf(stderr, "%c: %s\n", type_to_char(msg->type), msg->msg);
}
