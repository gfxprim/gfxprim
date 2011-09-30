%% extends "filter.point.c.t"

%% block descr
Invert filters -- Invert image
%% endblock

%% block body
#include <GP_Context.h>
#include <GP_Pixel.h>
#include <GP_GetPutPixel.h>

%% call(ps) filter_per_pixel_size('Invert')
pix = {{ 2 ** ps.size - 1 }} - pix;
%% endcall

%% call(chan) filter_per_pixel_type('Invert')
{{ chan[0] }} = {{ 2 ** chan[2] - 1 }} - {{ chan[0] }};
%% endcall

{{ filter_functions('Invert') }}

%% endblock body
