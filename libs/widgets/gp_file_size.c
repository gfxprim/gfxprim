//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdio.h>
#include <gp_file_size.h>

const char *gp_str_file_size(char *buf, size_t buf_len, size_t size)
{
	if (size < 1024)
		snprintf(buf, buf_len, "%zuB", size);
	else if (size < 1024 * 1024)
		snprintf(buf, buf_len, "%.2fKB", 1.00 * size/1024);
	else if (size < 1024 * 1024 * 1024)
		snprintf(buf, buf_len, "%.2fMB", 1.00 * size/(1024 * 1024));
	else if (size < 1024 * 1024 * 1024 * 1024LLU)
		snprintf(buf, buf_len, "%.2fGB", 1.00 * size/(1024 * 1024 * 1024));
	else
		snprintf(buf, buf_len, "%.2fTB", 1.00 * size/(1024 * 1024 * 1024 * 1024LLU));

	return buf;
}
