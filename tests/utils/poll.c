// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <unistd.h>
#include <utils/gp_poll.h>

#include "tst_test.h"

static int called;

static enum gp_poll_event_ret count_event(gp_fd *self)
{
	(void) self;

	called++;

	return GP_POLL_RET_OK;
}

static int open_pipe(int fds[2])
{
	if (pipe(fds)) {
		tst_msg("pipe() failed");
		return 1;
	}

	return 0;
}

/*
 * Removing the last fd closes the epoll instance, adding a fd afterwards has
 * to open a new one.
 */
static int poll_rem_add(void)
{
	gp_poll poll = {};
	int fds[2], ret = TST_FAILED;
	gp_fd fd = {
		.fd = -1,
		.event = count_event,
		.events = GP_POLLIN,
	};

	if (open_pipe(fds))
		return TST_UNTESTED;

	fd.fd = fds[0];
	called = 0;

	gp_poll_add(&poll, &fd);
	gp_poll_rem(&poll, &fd);

	if (poll.ep_fd != -1) {
		tst_msg("Epoll fd %i not reset after the last fd was removed", poll.ep_fd);
		goto exit;
	}

	gp_poll_add(&poll, &fd);

	if (write(fds[1], "a", 1) != 1) {
		tst_msg("write() failed");
		goto exit;
	}

	if (gp_poll_wait(&poll, 0)) {
		tst_msg("gp_poll_wait() failed");
		goto exit;
	}

	if (called != 1) {
		tst_msg("Callback called %i times expected 1", called);
		goto exit;
	}

	ret = TST_PASSED;
exit:
	gp_poll_rem(&poll, &fd);
	close(fds[0]);
	close(fds[1]);
	return ret;
}

const struct tst_suite tst_suite = {
	.suite_name = "Poll testsuite",
	.tests = {
		{.name = "Poll rem then add",
		 .tst_fn = poll_rem_add,
		 .flags = TST_CHECK_MALLOC},
		{.name = NULL},
	}
};
