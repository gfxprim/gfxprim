//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utils/gp_path.h>

char *gp_user_home(void)
{
	char *home_path;

	home_path = getenv("HOME");
	if (!home_path) {
		errno = ENOENT;
		return NULL;
	}

	if (home_path[0] != '/') {
		errno = EINVAL;
		return NULL;
	}

	return home_path;
}

#define PATH_DELIM '/'

static int next_component(const char **path, char *res,
                          enum gp_mkpath_flags flags)
{
	size_t len = 0, i = 0;

	while (**path == PATH_DELIM)
		(*path)++;

	while (**path && **path != PATH_DELIM && ++len < PATH_MAX)
		*(res++) = *((*path)++);

	if (len >= PATH_MAX)
		return -1;

	*res = 0;

	if (flags & GP_MKPATH_HAS_FILE) {
		while (*path[i] && *path[i] != PATH_DELIM)
			i++;

		if (!*path[i])
			return 0;
	}

	return len;
}

int gp_mkpath(const char *base_path, const char *path,
              enum gp_mkpath_flags flags, int mode)
{
	char buf[PATH_MAX];
	char dir[PATH_MAX];
	int dir_fd;
	int err;

	if (!base_path) {
		base_path = buf;

		if (next_component(&path, buf, flags) < 0) {
			err = ENAMETOOLONG;
			goto err0;
		}
	}

	dir_fd = open(base_path, O_DIRECTORY);
	if (dir_fd < 0) {
		err = errno;
		goto err0;
	}

	for (;;) {
		int ret = next_component(&path, dir, flags);

		if (ret < 0) {
			err = ENAMETOOLONG;
			goto err1;
		}

		if (!ret) {
			close(dir_fd);
			return 0;
		}

		int fd = openat(dir_fd, dir, O_DIRECTORY);

		if (fd == -1 && errno == ENOENT) {
			if (mkdirat(dir_fd, dir, mode)) {
				err = errno;
				goto err1;
			}

			fd = openat(dir_fd, dir, O_DIRECTORY);
		}

		if (fd == -1) {
			err = errno;
			goto err1;
		}

		close(dir_fd);
		dir_fd = fd;
	}
err1:
	close(dir_fd);
err0:
	errno = err;
	return -1;
}
