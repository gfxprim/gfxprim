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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "base.c.t"

{% block descr %}Table for fixed point cubic coeficients for A=0.5{% endblock %}

%% block body

/*
 * Fixed point cubic coeficients.
 *
 * Copyright (c) 2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdint.h>

int16_t GP_CubicTable[2047] = {
%% set A=0.5
%% for i in range(0, 1023)
%% set x = i/1024
	{{ round(((2 - A)*x*x*x + (A - 3)*x*x + 1) * 1024) }}, /* {{ i }} {{ x }} */
%% endfor
%% for i in range(1024, 2047)
%% set x = i/1024
	{{ round((-A*x*x*x + 5*A*x*x - 8*A*x + 4*A) * 1024) }}, /* {{ i }} {{ x }} */
%% endfor
};

%% endblock body
