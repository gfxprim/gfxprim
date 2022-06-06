// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef TEXT_GP_UTF_H
#define TEXT_GP_UTF_H

#include <stdint.h>

static inline uint32_t gp_utf8_next(const char **str)
{
	uint32_t s0 = *str[0];

	(*str)++;

	if ((s0 & 0x80) == 0x00)
		return s0;

	uint32_t s1 = *str[0];

	if ((s1 & 0xc0) != 0x80)
		return -1;

	s1 &= 0x3f;
	
	(*str)++;

	if ((s0 & 0xe0) == 0xc0)
		return (s0 & 0x1f)<<6 | s1;

	uint32_t s2 = *str[0];
	
	if ((s2 & 0xc0) != 0x80)
		return -1;

	s2 &= 0x3f;
       
	(*str)++;

	if ((s2 & 0xf0) == 0xe0)
		return (s0 & 0x0f)<<12 | s1<<6 | s2;

	(*str)++;
	
	uint32_t s3 = *str[0];
	
	if ((s3 & 0xc0) != 0x80)
		return -1;

	s3 &= 0x3f;
       
	if ((s0 & 0xf1) == 0xf0)
		return (s0 & 0x07)<<18 | s1<<12 | s2<<6 | s3;

	return -1;
}

#endif /* TEXT_GP_UTF_H */
