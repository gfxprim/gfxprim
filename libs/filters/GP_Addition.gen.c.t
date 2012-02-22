%% extends "filter.arithmetic.c.t"

%% block descr
Addition filter -- Addition of two bitmaps.
%% endblock

%% block body

{{ filter_arithmetic_include() }}

%% macro filter_op(chan_name, chan_size)
{{ chan_name }} = {{ chan_name }}_A + {{ chan_name }}_B;
{{ filter_clamp_val(chan_name, chan_size) }}
%% endmacro

%% call(pt) filter_arithmetic_per_channel('Addition', filter_op)
%% endcall

%% call(ps) filter_arithmetic_per_bpp('Addition', filter_op)
%% endcall

{{ filter_arithmetic_functions('Addition') }}

%% endblock body
