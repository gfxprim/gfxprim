%% extends "filter.point.c.t"

{% block descr %}Noise filter -- Adds noise to an image.{% endblock %}

%% block body

{{ filter_point_include() }}

%% macro filter_op(chann_name, chann_size)
{{ chann_name }} = {{ chann_name }} + (random() % ({{ chann_name }}_max * 2)) - {{ chann_name }}_max;
{{ filter_clamp_val(chann_name, chann_size) }}
%% endmacro

/*
 * Generated noise filters.
 */
%% call(pt) filter_point_per_channel('Noise', 'GP_FilterParam ratio[]', filter_op)
{{ filter_params(pt, 'ratio', 'float ', '_rat', 'f') }}
%% for chann in pt.chanslist
	int {{ chann[0] }}_max = {{ chann[0] }}_rat * {{ 2 ** chann[2] - 1}} + 0.5;
%% endfor
%% endcall

/*
 * Generated noise filters for pixels with one channel.
 */
%% call(ps) filter_point_per_bpp('Noise', 'GP_FilterParam ratio[]', filter_op)
{{ filter_param(ps, 'ratio', 'float ', '_rat', 'f') }}
	int pix_max = pix_rat * {{ 2 ** ps.size - 1}} + 0.5;
%% endcall

{{ filter_functions('Noise', 'GP_FilterParam ratio[]', 'ratio') }}

%% endblock body
