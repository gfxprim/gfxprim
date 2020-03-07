// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef __SPIV_HELP_H__
#define __SPIV_HELP_H__

/*
 * Prints help into stderr
 */
void print_help(void);

void print_man(void);

/*
 * Draws help, waits for keypress.
 */
void draw_help(gp_backend *backend);

#endif /* __SPIV_HELP_H__ */
