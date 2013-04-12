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
 * Copyright (C) 2012      Cyril Hrubis    <metan@ucw.cz>                    *
 *                                                                           *
 *****************************************************************************/

%% extends "base.c.t"

{% block descr %}Gamma correction tables for Gamma = 2.2{% endblock %}

%% block body

/*
 * Gamma correction tables.
 *
 * Copyright (c) 2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdint.h>

/*
 * Converts 8 bit gamma to 10 bit linear.
 */
static uint16_t gamma8_linear10[] = {
%% for i in range(0, 256)
	{{ int(((float(i)/255) ** 2.2) * 1024 + 0.5) }}, /* {{i}} */
%% endfor
};

/*
 * Converts 10 bit linear to 8 bit gamma.
 */
static uint8_t linear10_gamma8[] = {
%% for i in range(0, 1025)
	{{ int(((float(i)/1024) ** (1/2.2)) * 255 + 0.5) }}, /* {{i}} */
%% endfor
};

/*
 * Pointers to tables
 */
uint16_t *GP_Gamma8_Linear10 = gamma8_linear10;
uint8_t  *GP_Linear10_Gamma8 = linear10_gamma8;

%% endblock body
