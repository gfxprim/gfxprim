#!/bin/bash
#
# Run dynamically linked test.
#

PROG="$1"
shift

LD_LIBRARY_PATH=../../build/ ./$PROG "$@"
