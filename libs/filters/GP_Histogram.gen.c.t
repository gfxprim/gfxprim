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

%% extends "filter.stats.c.t"

{% block descr %}Histogram filter -- Compute image histogram{% endblock %}

%% block body

{{ filter_stats_include() }}

%% macro filter_op(chan_name, chan_size)
{{ chan_name }}_hist->hist[{{ chan_name }}]++;
%% endmacro

%% call(pt) filter_point_per_channel('Histogram', 'GP_FilterParam histogram[]', filter_op)
{{ filter_params(pt, 'histogram', 'GP_Histogram *', '_hist', 'ptr') }}
%% endcall

%% call(ps) filter_point_per_bpp('Histogram', 'GP_FilterParam histogram[]', filter_op)
{{ filter_param(ps, 'histogram', 'GP_Histogram *', '_hist', 'ptr') }}
%% endcall

{{ filter_functions('Histogram', 'GP_FilterParam histogram[]', 'histogram') }}

%% endblock body
