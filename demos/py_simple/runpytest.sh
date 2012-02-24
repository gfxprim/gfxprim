#!/bin/bash
#
# Run python test with dynamically linked libGP.
#

PROG="$1"
shift

LD_LIBRARY_PATH=../../build/ PYTHONPATH=$PYTHONPATH:../../pylib/ ./$PROG "$@"
