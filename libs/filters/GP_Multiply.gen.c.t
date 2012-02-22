%% extends "filter.arithmetic.c.t"

%% block descr
Multiply filter -- Multiplies two images.
%% endblock

%% block body

{{ filter_arithmetic_include() }}

%% macro filter_op(chan_name, chan_size)
{{ chan_name }} = ({{ chan_name }}_A * {{ chan_name }}_B + {{ (2 ** chan_size - 1) // 2}})/ ({{ 2 ** chan_size - 1}});
{{ filter_clamp_val(chan_name, chan_size) }}
%% endmacro

%% call(pt) filter_arithmetic_per_channel('Multiply', filter_op)
%% endcall

%% call(ps) filter_arithmetic_per_bpp('Multiply', filter_op)
%% endcall

{{ filter_arithmetic_functions('Multiply') }}

%% endblock body
