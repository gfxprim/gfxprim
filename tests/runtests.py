#!/usr/bin/env python
#
# This is a simple glue script to run the tests and collect the results
#

#
# We expect simple structure i.e. the tests_dir contains directories with
# tests, each directory that contains tests has test_list.txt with a list of
# tests to run. The structure is then mirrored in the results_dir filled with
# json logs.
#

import os
import datetime

#
# Relative path to root directory containing tests
#
tests_dir='.'

#
# Results directory prefix
#
results_dir='results'

#
# Relative path to the directory with GP libraries to run tests against
#
build_dir='../build'

#
# By default the glibc __libc_message() writes to /dev/tty before calling
# the abort(). Exporting this macro makes it to use stderr instead.
#
# The main usage of the function are malloc assertions, so this makes us catch
# the malloc error message by catching stderr output.
#
runline_prep='export LIBC_FATAL_STDERR_=1;'

#
# Relative path, from the current directory, to the framework preload library.
#
framework_lib='framework/libtst_preload.so'

debug = 0

def globpath(path):
    return os.getcwd() + '/' + path

#
# Reads test_list.txt test file and executes tests one after another
#
def run_test(resdir, tstdir, runtest):
    f = open(runtest, 'r')
    lines = f.readlines()
    f.close()

    for line in lines:
        # ignore comments
        if (line[0] == '#'):
            continue
        # and blanks
        if (line.isspace()):
            continue

        line = line.strip();

        #
        # This is a little hairy but what it does is to constructs correct
        # paths to preload and dynamic libraries and runs the test. 
        #
        runline  = runline_prep + ' '
        runline += 'export LD_PRELOAD="' + globpath(framework_lib) + '"; '
        runline += 'export LD_LIBRARY_PATH="' + globpath(build_dir) + '"; '
        runline += 'cd ' + tstdir + ' && ./' + line + ' -o "' + globpath(resdir) + '"'

        if debug >= 2:
            print(" LINE: %s" % runline)

        os.system(runline)

#
# Discovers tests in directories.
#
def run_tests(resdir, testsdir):

    if debug >= 1:
        print('Looking for tests in "%s"' % testsdir)

    for root, dirs, _ in os.walk(testsdir):
        for name in dirs:

            path = root + '/' + name

            if debug >= 2:
                print('Looking into dir "%s"' % path)

            runtest = path + '/test_list.txt'

            if (os.access(runtest, os.R_OK)):
                # Create result directory
                curresdir = resdir + '/' + name
                os.mkdir(curresdir)
                # Run tests
                print("\n========= Running " + name + " testsuites =========\n")
                run_test(curresdir, path, runtest)

def main():
    now = datetime.datetime.now() 
    resdir = '%s_%i-%02i-%02i_%02i-%02i-%02i' % (results_dir, now.year, now.month,
                                       now.day, now.hour, now.minute, now.second)
    print('Creating result directory "%s"' % resdir)
    os.mkdir(resdir)

    run_tests(resdir, tests_dir)

if __name__ == '__main__':
    main()
