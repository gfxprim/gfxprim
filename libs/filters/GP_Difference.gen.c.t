%% extends "filter.arithmetic.c.t"

%% block descr
Difference filter -- Symetric difference of two bitmaps.
%% endblock

%% block body

{{ filter_arithmetic_include() }}

%% macro filter_op(chan_name, chan_size)
{{ chan_name }} = GP_ABS({{ chan_name }}_A - {{ chan_name }}_B);
%% endmacro

%% call(pt) filter_arithmetic_per_channel('Difference', filter_op)
%% endcall

%% call(ps) filter_arithmetic_per_bpp('Difference', filter_op)
%% endcall

{{ filter_arithmetic_functions('Difference') }}

%% endblock body
