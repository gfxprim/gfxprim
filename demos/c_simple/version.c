// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

/*

  Prints GFXprim version.

 */

#include <gfxprim.h>

int main(void)
{
	printf("GFXprim verson %i.%i.%i (%s)\n",
	       GP_VER_MAJOR, GP_VER_MINOR, GP_VER_MICRO, GP_VER_STR);

	return 0;
}
