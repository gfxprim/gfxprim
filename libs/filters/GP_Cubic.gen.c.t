@ include source.t
/*
 * Table for fixed point cubic coeficients for A=0.5
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdint.h>

int16_t gp_cubic_table[2047] = {
@ A=0.5
@ for i in range(0, 1023):
@     x = i/1024
	{{ round(((2 - A)*x*x*x + (A - 3)*x*x + 1) * 1024) }}, /* {{ '%-4i %.6f' % (i, x) }} */
@ for i in range(1024, 2047):
@     x = i/1024
	{{ round((-A*x*x*x + 5*A*x*x - 8*A*x + 4*A) * 1024) }}, /* {{ '%-4i %.6f' % (i, x) }} */
@ end
};
