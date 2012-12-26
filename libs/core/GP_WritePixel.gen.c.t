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

{% block description %}Write pixel{% endblock %}

{% block body %}

#include "core/GP_GetSetBits.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_WritePixel.gen.h" 

{# Some pixel types has hand written optimized functions #}
%% set hand_optimized = ['1BPP_LE', '1BPP_BE',
                         '2BPP_LE', '2BPP_BE',
                         '4BPP_LE', '4BPP_BE',
                         '8BPP', '16BPP',
                         '24BPP', '32BPP']

%% for ps in pixelsizes
%% if ps.suffix not in hand_optimized
%% if ps.needs_bit_endian()
void GP_WritePixels_{{ ps.suffix }}(void *start, uint8_t off,
                            size_t cnt, unsigned int val)
{
	//TODO:
}
%% else
void GP_WritePixels_{{ ps.suffix }}(void *start, size_t cnt, unsigned int val)
{
	//TODO:
}
%% endif

%% endif
%% endfor

{% endblock body %}
