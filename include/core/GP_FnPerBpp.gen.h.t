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
 * Copyright (C) 2011-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends 'base.h.t'

{% block descr %}All FnPerBpp macros{% endblock %}

{% block body %}

/*
 * Macros used to create draving functions from macros.
 */
#define GP_DEF_FN_PER_BPP(fname, MACRO_NAME, fdraw) \
%% for ps in pixelsizes
	GP_DEF_FN_FOR_BPP(fname, MACRO_NAME, fdraw, {{ ps.suffix }}) \
{% endfor %}

{% macro bpp_suffix(suffix) %}{% if suffix == "LE" or suffix == "BE" %}_{{ suffix }}{% endif %}{% endmacro %}

/*
 * Branch on bpp and bit_endian.
 */
#define GP_FN_PER_BPP(FN_NAME, bpp, bit_endian, ...) \
	switch (bpp) { \
%% for bpp in pixelsizes_by_bpp.keys()
	case {{ bpp }}: \
%% if len(pixelsizes_by_bpp[bpp]) == 1
		FN_NAME##_{{ bpp }}BPP{{ bpp_suffix(pixelsizes_by_bpp[bpp][0]) }}(__VA_ARGS__); \
%% else
		if (bit_endian == GP_BIT_ENDIAN_LE) \
			FN_NAME##_{{ bpp }}BPP_LE(__VA_ARGS__); \
		else \
			FN_NAME##_{{ bpp }}BPP_BE(__VA_ARGS__); \
%% endif
	break; \
%% endfor
	}

/*
 * Branch on bpp and bit_endian.
 */
#define GP_FN_RET_PER_BPP(FN_NAME, bpp, bit_endian, ...) \
	switch (bpp) { \
%% for bpp in pixelsizes_by_bpp.keys()
	case {{ bpp }}: \
%% if len(pixelsizes_by_bpp[bpp]) == 1
		return FN_NAME##_{{ bpp }}BPP{{ bpp_suffix(pixelsizes_by_bpp[bpp][0]) }}(__VA_ARGS__); \
%% else
		if (bit_endian == GP_BIT_ENDIAN_LE) \
			return FN_NAME##_{{ bpp }}BPP_LE(__VA_ARGS__); \
		else \
			return FN_NAME##_{{ bpp }}BPP_BE(__VA_ARGS__); \
%% endif
	break; \
%% endfor
	}

{% endblock body %}
