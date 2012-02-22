%% extends "filter.point.c.t"

%% block descr
Invert filters -- Invert image
%% endblock

%% block body

{{ filter_point_include() }}

%% macro filter_op(chann_name, chann_size)
{{ chann_name }} = {{ 2 ** chann_size - 1 }} - {{ chann_name }};
%% endmacro

%% call(pt) filter_point_per_channel('Invert', '', filter_op)
%% endcall

%% call(ps) filter_point_per_bpp('Invert', '', filter_op)
%% endcall

{{ filter_functions('Invert') }}

%% endblock body
