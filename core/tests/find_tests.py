#!/bin/python

import sys, re, getopt

# Helpers

def die(msg_s, fname=None, line=None):
  msg('E', msg_s, fname, line)
  sys.exit(1)

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

# Globals

suites = {} # {"suitename":["testname":{test_parameters}]}

suite_re = re.compile("\A\\s*GP_SUITE\\((.*)\\)\\s*\Z")
test_re = re.compile("\A\\s*GP_TEST\\((.*)\\)\\s*\Z")


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

def gen_tests(f):
  f.write("/*\n"
	  " * This GENERATED file is part of GfxPrim. See LICENSE for legal stuff.\n"
	  " * Do not modify this file directly!\n"
	  " */\n\n"
	  "#include <check.h>\n\n")

  for suite, tests in suites.iteritems():
    f.write("/****************************************************************\n"
	    " * Suite %s\n"
	    " */\n\n" % suite)
    for t in tests:
      f.write("/*\n"
	      " * Test %s/%s defined in %s:%d\n"
	      " */\n\n" % (suite, t['name'], t['fname'], t['line']))
      f.write("void GP_TEST_%s(int);\n\n"
	      "TCase *GP_TC_%s_%s()\n"
	      "{\n"
	      "	TCase *tc = tcase_create(\"%s\");\n" %
	      (t['name'], suite, t['name'], t['name']))
      signal = t.get('expect_signal', 0)
      exitval = t.get('expect_exit', 0)
      assert ('loop_start' in t) == ('loop_end' in t)
      loop_start = t.get('loop_start', 0)
      loop_end = t.get('loop_end', 1)
      f.write("	_tcase_add_test(tc, GP_TEST_%s, \"%s\", %d, %d, %d, %d);\n" % \
	      (t['name'], t['name'], signal, exitval, loop_start, loop_end))
      f.write("	return tc;\n}\n\n")
      # TODO: Handle special test requirements (timing, fixture, ...)

    f.write("Suite *GP_TS_%s()\n"
	    "{\n"
	    "	Suite *s = suite_create(\"%s\");\n" % (suite, suite))
    for t in tests:
      f.write("	suite_add_tcase(s, GP_TC_%s_%s());\n" % (suite, t['name']))
    f.write("	return s;\n"
	    "}\n\n")

  f.write("/****************************************************************\n"
	  " * Create and add all suites to a SRunner\n"
	  " */\n\n"
	  "void GP_AddSuitesToSRunner(SRunner *sr)\n"
	  "{\n")
  for suite in suites:
    f.write("	srunner_add_suite(sr, GP_TS_%s());\n" % suite)
  f.write("}\n\n")


def main():
  try:
    opts, args = getopt.gnu_getopt(sys.argv[1:], "c:")
  except:
    die("Invalid arguments")
  cfile = None
  for opt, val in opts:
    if opt == '-c':
      cfile = val
  if not cfile:
    die("You must specify: -c <output.c>")

  for fn in args:
    try:
      f = open(fn, 'rt')
    except:
      die("Unable to open '%s'" % fn)
    find_tests(f, fn)
    f.close()
    
  try:
    f = open(cfile, 'wt')
  except:
    die("Unable to open '%s'" % cfile)
  gen_tests(f)
  f.close()

if __name__ == '__main__':
  main()
