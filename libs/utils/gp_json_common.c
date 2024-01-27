// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021-2022 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <utils/gp_json_common.h>

void gp_json_err_handler(void *err_print_priv, const char *line)
{
	fputs(line, err_print_priv);
	putc('\n', err_print_priv);
}
