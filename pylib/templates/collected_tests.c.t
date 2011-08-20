%% extends "base.c.t"

%% block descr
Code creating the tests and suites for tests collected from .test.c files.
%% endblock

%% block body

#include <check.h>

%% for suite, tests in suites.items()

/****************************************************************
 * Suite {{ suite }}
 */

%% for t in tests
/*
 * Test {{ suite }}/{{ t['name'] }} defined in {{ t['fname'] }}:{{ t['line'] }}
 */

void GP_TEST_{{ t['name'] }}(int);

// TODO: Handle special test requirements (timing, fixture, ...)
TCase *GP_TC_{{ suite }}_{{ t['name'] }}()
{
	TCase *tc = tcase_create("{{ t['name'] }}");
	_tcase_add_test(tc, GP_TEST_{{ t['name'] }}, "{{ t['name'] }}",
		{{ t.get('expect_signal', 0) }}, {{ t.get('expect_exit', 0) }},
		{{ t.get('loop_start', 0) }}, {{ t.get('loop_end', 1) }});
	return tc;
}

%% endfor

Suite *GP_TS_{{ suite }}() {
	Suite *s = suite_create("{{ suite }}");
%% for t in tests
	suite_add_tcase(s, GP_TC_{{ suite }}_{{ t['name'] }}());
%% endfor
	return s;
}

%% endfor

/****************************************************************
 * Create and add all suites to a SRunner
 */
void GP_AddSuitesToSRunner(SRunner *sr) {
%% for suite, tests in suites.items()
	srunner_add_suite(sr, GP_TS_{{ suite }}());
%% endfor
}

%% endblock body
