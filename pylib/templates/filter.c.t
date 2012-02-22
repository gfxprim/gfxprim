%% extends "common.c.t"

%% macro filter_include()
#include "core/GP_Context.h"
#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "core/GP_Debug.h"
#include "GP_Filter.h"
%% endmacro

/*
 * Value clamping macro
 */
%% macro filter_clamp_val(var, size)
			if ({{ var }} < 0)
				{{ var }} = 0;

			if ({{ var }} > {{ 2 ** size - 1}})
				{{ var }} = {{ 2 ** size - 1}};
%% endmacro 

/*
 * Load parameters from params structure into variables
 */
%% macro filter_params(pt, params, c_type, suffix, id)
	GP_ASSERT(GP_FilterParamCheckPixelType({{ params }}, GP_PIXEL_{{ pt.name }}) == 0,
	          "Invalid params channels for context pixel type");
	
	%% for chann in pt.chanslist
	{{ c_type }}{{ chann[0] }}{{ suffix }} = (GP_FilterParamChannel({{ params }}, "{{ chann[0] }}"))->val.{{ id }};
	%% endfor
%% endmacro

%% macro filter_params_raw(pt, params, suffix)
	GP_ASSERT(GP_FilterParamCheckPixelType({{ params }}, GP_PIXEL_{{ pt.name }}) == 0,
	          "Invalid params channels for context pixel type");
	
	%% for chann in pt.chanslist
	GP_FilterParam *{{ chann[0] }}{{ suffix }} = GP_FilterParamChannel({{ params }}, "{{ chann[0] }}");
	%% endfor
%% endmacro

/*
 * Load parameters from params structure into variable
 */
%% macro filter_param(ps, params, c_type, suffix, id)
	GP_ASSERT(GP_FilterParamChannels({{ params }}) != 1,
	          "Expected only one channel");

	{{ c_type }}pix{{ suffix }} = {{ params }}[0].val.{{ id }};
%% endmacro

%% macro filter_param_raw(ps, params, suffix)
	GP_ASSERT(GP_FilterParamChannels({{ params }}) != 1,
	          "Expected only one channel");

	GP_FilterParam *pix{{ suffix }} = &{{ params }}[0];
%% endmacro
