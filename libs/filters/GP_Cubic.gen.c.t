%% extends "base.c.t"

{% block descr %}Table for fixed point cubic coeficients for A=0.5{% endblock %}

%% block body

/*
 * Fixed point cubic coeficients.
 *
 * Copyright (c) 2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdint.h>

int16_t GP_CubicTable[2047] = {
%% set A=0.5
%% for i in range(0, 1023)
%% set x = i/1024
	{{ round(((2 - A)*x*x*x + (A - 3)*x*x + 1) * 1024) }}, /* {{ i }} {{ x }} */
%% endfor
%% for i in range(1024, 2047)
%% set x = i/1024
	{{ round((-A*x*x*x + 5*A*x*x - 8*A*x + 4*A) * 1024) }}, /* {{ i }} {{ x }} */
%% endfor
};

%% endblock body
