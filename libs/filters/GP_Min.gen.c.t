%% extends "filter.arithmetic.c.t"

%% block descr
Min filter -- Choose min for each pixel (and for each channel) for two bitmaps.
%% endblock

%% block body

{{ filter_arithmetic_include() }}

%% macro filter_op(chan_name, chan_size)
{{ chan_name }} = GP_MIN({{ chan_name }}_A, {{ chan_name }}_B);
%% endmacro

%% call(pt) filter_arithmetic_per_channel('Min', filter_op)
%% endcall

%% call(ps) filter_arithmetic_per_bpp('Min', filter_op)
%% endcall

{{ filter_arithmetic_functions('Min') }}

%% endblock body
