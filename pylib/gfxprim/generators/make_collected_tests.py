#!/usr/bin/python
#
# Script generating collected_tests.gen.c
#
# Scrapes the target directory for .test.c files, looks for 
# GP_TEST and GP_SUITE macros and generates code creating all the 
# tests and the suite
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz> 
#

# Also fixed in tests.mk
collected_tests_file = 'collected_tests.gen.c'

import os, re, glob
from gfxprim.generators.generator import *

def warn(msg_s, fname=None, line=None):
  msg('W', msg_s, fname, line)

def msg(prefix, msg_s, fname=None, line=None):
  s = '[' + prefix
  if fname:
    s += ' ' + fname
    if line:
      s += ':%d'%line
  s += '] '
  sys.stderr.write(s + msg_s + '\n')

testfile_pattern = '*.test.c'

suites = {} # {"suitename":["testname":{test_parameters}]}

suite_re = re.compile("\A\\s*GP_SUITE\\((.*)\\)\\s*\Z")
test_re = re.compile("\A\\s*GP_TEST\\((.*)\\)\\s*\Z")

@generator(CHeaderGenerator(name = collected_tests_file),
           descr = 'Code creating the tests and suites for tests collected '
	           'from .test.c files',
           authors = ["2011 - Tomas Gavenciak <gavento@ucw.cz>"])
def tests_collected_tests(c):
  fnames = glob.fnmatch.filter(os.listdir(c.fdir or '.'), testfile_pattern)
  print fnames
  for fn in fnames:
    dirfn = os.path.join(c.fdir, fn)
    with open(dirfn, 'rt') as f:
      find_tests(f, fn)
  if not fnames:
    warn('No .test.c files found in "%s".' % c.fdir)
  if not suites:
    warn('No suites found, generating empty testsuite.')
  
  c.rhead("#include <check.h>\n\n")

  for suite, tests in suites.iteritems():
    c.rbody(
        "/****************************************************************\n"
        " * Suite {{ suite }}\n"
        " */\n\n", suite=suite)
    for t in tests:
      assert ('loop_start' in t) == ('loop_end' in t)
      c.rbody(
	"/*\n"
	" * Test {{ suite }}/{{ t['name'] }} defined in {{ t['fname'] }}:{{ t['line'] }}\n"
	" */\n\n"
	"void GP_TEST_{{ t['name'] }}(int);\n\n"
	"TCase *GP_TC_{{ suite }}_{{ t['name'] }}() {\n"
	"	TCase *tc = tcase_create(\"{{ t['name'] }}\");\n"
        "	_tcase_add_test(tc, GP_TEST_{{ t['name'] }}, \"{{ t['name'] }}\", "
	           "{{ t.get('expect_signal', 0) }}, {{ t.get('expect_exit', 0) }}, "
	           "{{ t.get('loop_start', 0) }}, {{ t.get('loop_end', 1) }});\n"
	"	return tc;\n}\n\n", t=t, suite=suite)
	# TODO: Handle special test requirements (timing, fixture, ...)

    c.rbody(
        "Suite *GP_TS_{{ suite }}() {\n"
        "	Suite *s = suite_create(\"{{ suite }}\");\n"
	"{% for t in tests %}"
        "	suite_add_tcase(s, GP_TC_{{ suite }}_{{ t['name'] }}());\n"
	"{% endfor %}"
        "	return s;\n}\n\n", tests=tests, suite=suite)

  # Once for all suites
  c.rbody(
	"/****************************************************************\n"
	" * Create and add all suites to a SRunner\n"
	" */\n\n"
	"void GP_AddSuitesToSRunner(SRunner *sr) {\n"
	"{% for s in suites %}"
	"	srunner_add_suite(sr, GP_TS_{{ s }}());\n"
	"{% endfor %}"
	"}\n\n", suites=suites)

def find_GP_directive(name, regexp, l, fname='unknown', line=0):
  "Looks for a given GP_* directive, parses args if any, "
  "retuns (name, dict_of_args) or (None, None) if not found."
  if name in l:
    m = regexp.search(l)
    if not m:
      warn("found unsuitable %s directive, ignoring." % name, fname, line)
    else:
      d = m.groups()[0].split(',', 1)
      args = {}
      if len(d)>1:
	try:
	  s = 'dict( ' + d[1].strip(" \t\n\"") + ' )'
	  args = eval(s)
	except:
	  die("error parsing arguments: %r" % s, fname, line)
      return d[0].strip(), args
  return None, None

def find_tests(f, fname):
  "Finds all tests in a file."
  suite = None
  ls = f.readlines()
  for i in range(len(ls)):
    l = ls[i]
    # Look for suite declaration
    name, args = find_GP_directive("GP_SUITE", suite_re, l, fname=fname, line=i)
    if name:
      suite = name
      if args:
	warn("suite should have no arguments other than name.", fname, i)
      suites.setdefault(name, [])
    # Look for suite declaration
    name, args = find_GP_directive("GP_TEST", test_re, l, fname=fname, line=i)
    if name:
      test_suite = suite
      if 'suite' in args:
	test_suite = args['suite']
      if not test_suite:
	test_suite = 'default'
	warn("no suite defined before test %s, using %r." % (name, test_suite), fname, i)
      args['name'] = name
      args['fname'] = fname
      args['line'] = i
      suites.setdefault(test_suite, [])
      suites[test_suite].append(args)

