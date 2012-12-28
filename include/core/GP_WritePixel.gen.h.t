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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends 'base.h.t'

{% block description %}Write Pixels generated header{% endblock %}

{% block body %}

/*
 * These functions writes cnt pixels using value val starting at start address
 * and additionally (for pixel sizes that are not aligned to the whole bytes)
 * at off offset in the first byte (i.e. byte at the start address).
 */

%% for ps in pixelsizes
%% if ps.needs_bit_endian()
void GP_WritePixels_{{ ps.suffix }}(void *start, uint8_t off,
                            size_t cnt, unsigned int val);

%% else
void GP_WritePixels_{{ ps.suffix }}(void *start, size_t cnt, unsigned int val);

%% endif
%% endfor

{% endblock body %}
