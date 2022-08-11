#!/bin/sh
#
# Start afl in NCPU + 1 threads to saturate CPU
#

NCPU=$(getconf _NPROCESSORS_ONLN)

DATADIR=images/
OUTDIR=result/
EXEC="./loaders @@"
export LD_LIBRARY_PATH=../../build/

for i in $(seq -w 2 $NCPU); do
	afl-fuzz -t 1000 -D -i "$DATADIR" -o "$OUTDIR" -S fuzzer$i $EXEC > /dev/null&
done

afl-fuzz -t 1000 -D -i "$DATADIR" -o "$OUTDIR" -M fuzzer01 $EXEC
