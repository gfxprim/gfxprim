// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

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

#define ERR(err) \
	case err: \
		return #err \

const char *tst_strerr(int err)
{
	switch (err) {
	case 0:
		return "SUCCESS";
	/* asm-generic/errno-base.h */
	ERR(EPERM);
	ERR(ENOENT);
	ERR(ESRCH);
	ERR(EINTR);
	ERR(EIO);
	ERR(ENXIO);
	ERR(E2BIG);
	ERR(ENOEXEC);
	ERR(EBADF);
	ERR(ECHILD);
	ERR(EAGAIN);
	ERR(ENOMEM);
	ERR(EACCES);
	ERR(EFAULT);
	ERR(ENOTBLK);
	ERR(EBUSY);
	ERR(EEXIST);
	ERR(EXDEV);
	ERR(ENODEV);
	ERR(ENOTDIR);
	ERR(EISDIR);
	ERR(EINVAL);
	ERR(ENFILE);
	ERR(EMFILE);
	ERR(ENOTTY);
	ERR(ETXTBSY);
	ERR(EFBIG);
	ERR(ENOSPC);
	ERR(ESPIPE);
	ERR(EROFS);
	ERR(EMLINK);
	ERR(EPIPE);
	ERR(EDOM);
	ERR(ERANGE);

	/* a few from asm-generic/errno.h */
	ERR(EDEADLK);
	ERR(ENAMETOOLONG);
	ERR(ENOSYS);
	ERR(ECANCELED);
	default:
		return "???";
	}
}
