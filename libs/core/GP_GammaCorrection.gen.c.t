%% extends "base.c.t"

{% block descr %}Gamma correction tables for Gamma = 2.2{% endblock %}

%% block body

/*
 * Gamma correction tables.
 *
 * Copyright (c) 2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdint.h>

/*
 * 8-bit linear to gamma translation table.
 */
static uint8_t linear_to_gamma_8bit[] = {
%% for i in range(0, 256)
	{{ int(((float(i)/255) ** (1/2.2)) * 255 + 0.5) }}, /* {{i}} */
%% endfor
};

/*
 * Pointer to 8 bit translation table.
 */
uint8_t *GP_LinearToGamma_8bit = linear_to_gamma_8bit;


%% endblock body
