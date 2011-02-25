#!/bin/sh
#
# Run dynamically linked test.
#

PROG="$1"
shift

echo "LD_LIBRARY_PATH=../:../../core/ ./$PROG $@"
LD_LIBRARY_PATH="../:../../core/" ./$PROG $@
