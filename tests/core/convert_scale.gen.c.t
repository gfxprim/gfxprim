@ include source.t
/*
 * ConvertScale tests.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include <core/gp_convert_scale.gen.h>

#include "tst_test.h"

@ max_in = 24
@ max_out = 16
@
@ for i in range(1, max_in):
@     for j in range(1, max_out):
static int check_convert_{{ i }}_{{ j }}(void)
{
	unsigned int v, fail = 0;
	uint32_t res, exp_res;
	float fres;

	for (v = 0; v < {{ 2 ** i - 1 }}; v++) {
		res = GP_SCALE_VAL_{{ i }}_{{ j }}(v);
@         if j > i:
		/*
		 * We have {{ 2**i }} values and we need to map them to
		 * subset of {{ 2**j }} values while making sure 0 -> 0
		 * and {{ 2**i - 1 }} -> {{ 2**j - 1 }} and that the
		 * mapping is as evenly distributed as possible.
		 *
		 * So we map the input to 0-1 interval by dividing it by
		 * maximal input value {{ 2**i - 1 }} and then multiply
		 * it by output maximal value {{ 2**j - 1}}.
		 */
		fres = (v / {{ (2.00 ** i - 1) }}) * {{ (2.00 ** j - 1) }};
		exp_res = round(fres);
@         else:
		/*
		 * We have {{ 2**i }} values that must be mapped to {{ 2**j }}
		 * so we do simple division and floor() which maps the values
		 * evenly, 0 -> 0 and {{ 2**i - 1 }} -> {{ 2**j - 1 }}.
		 *
		 * In terms for implementation this is just bitshift.
		 */
		fres = v * {{ (2.00 ** j) / (2.00 ** i) }};
		exp_res = floor(fres);
@         end

		if (res != exp_res) {
			if (fail < 5)
				tst_msg("GP_SCALE_{{ i }}_{{ j }}(%i) = %i, "
				        "expected %i %f", v, res, exp_res, fres);
			fail++;
		}
	}

	if (fail) {
		if (fail > 5)
			tst_msg("+ next %u failures", fail - 5);
		return TST_FAILED;
	}

	return TST_PASSED;
}

@ endfor
@
const struct tst_suite tst_suite = {
	.suite_name = "Convert Scale Testsuite",
	.tests = {
@ for i in range(1, max_in):
@     for j in range(1, max_out):
		{.name = "SCALE_{{ i }}_{{ j }}()",
		 .tst_fn = check_convert_{{ i }}_{{ j }}},
@ end
		{.name = NULL}
	}
};
