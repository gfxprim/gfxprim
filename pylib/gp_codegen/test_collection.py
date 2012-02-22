#
# Scrapes the target directory for .test.c files, looks for
# GP_TEST and GP_SUITE macros and generates code creating all the
# tests and the suite
#
# 2011 - Tomas Gavenciak <gavento@ucw.cz>
#

import os
import re
import glob
import logging as log
from .render_utils import create_environment, render_file

testfile_patterns = ['*.test.c', '*.test.gen.c']

suite_re = re.compile("\A\\s*GP_SUITE\\((.*)\\)\\s*\Z")
test_re = re.compile("\A\\s*GP_TEST\\((.*)\\)\\s*\Z")

# Also fixed in tests.mk
collected_tests_file = 'collected_tests.gen.c'

# suites is a dict of tests:
#   {"suitename": [Test]}

# Test is a dict:
#   {"attribute": val}
# attributes are:
#   name, fname, line

def find_tests(fname, suites):
  "Finds all tests in a file, extend suites."
  suite = None
  f = open(fname, 'rt')
  ls = list(f.readlines())
  f.close()
  for i in range(len(ls)):
    l = ls[i]
    # Look for suite declaration
    name, args = find_GP_directive("GP_SUITE", suite_re, l, fname=fname, line=i)
    if name:
      if args:
	log.warning("%s:%s: Suite should have no arguments other than name.", fname, i)
      suites.setdefault(name, [])
      suite = name
    # Look for test declaration
    name, args = find_GP_directive("GP_TEST", test_re, l, fname=fname, line=i)
    if name:
      test_suite = suite
      if 'suite' in args:
	test_suite = args['suite']
      if not test_suite:
	test_suite = 'default'
	log.warning("%s:%s: No suite defined before test %s, using %r.", fname, i, name, test_suite)
      args['name'] = name
      args['fname'] = fname
      args['line'] = i
      assert ('loop_start' in args) == ('loop_end' in args)
      suites.setdefault(test_suite, [])
      suites[test_suite].append(args)


def collect_suites(fdir):
  fnames = []
  suites = {}
  for pat in testfile_patterns:
    fnames += glob.fnmatch.filter(os.listdir(fdir), pat)
  for fn in fnames:
    find_tests(os.path.join(fdir, fn), suites)
  if not fnames:
    log.warning('No .test.c files found in "%s".', fdir)
  if not suites:
    log.warning('No suites found, generating an empty testsuite.')
  return suites


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
	  log.fatal("%s:%s: error parsing arguments: %r", fname, line, s)
      return d[0].strip(), args
  return None, None


