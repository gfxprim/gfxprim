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
 * Copyright (C) 2012      Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "base.h.t"

{% block descr %}Gamma corrections.{% endblock %}

%% block body


extern uint16_t *GP_Gamma8_Linear10;
extern uint8_t  *GP_Linear10_Gamma8;

%% for i in range(1, 9)
static inline uint16_t GP_Gamma{{ i }}ToLinear10(uint8_t val)
{
	return GP_Gamma8_Linear10[val<<{{8 - i}}];
}

%% endfor

%% for i in range(1, 9)
static inline uint8_t GP_Linear10ToGamma{{ i }}(uint16_t val)
{
	return (GP_Linear10_Gamma8[val] + {{ int(2 ** (7 - i))}})>>{{8 - i}};
}

%% endfor

%% endblock body
