%% extends "filter.stats.c.t"

%% block descr
Histogram filter -- Compute image histogram.
%% endblock

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
