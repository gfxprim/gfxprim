%% extends "filter.point.c.t"

%% block descr
Invert filters -- Invert image
%% endblock

%% block body

{{ filter_include() }}

%% call(ps) filter_per_pixel_size('Invert')
pix = {{ 2 ** ps.size - 1 }} - pix;
%% endcall

%% call(chan) filter_per_pixel_type('Invert')
{{ chan[0] }} = {{ 2 ** chan[2] - 1 }} - {{ chan[0] }};
%% endcall

{{ filter_functions('Invert') }}

%% endblock body
