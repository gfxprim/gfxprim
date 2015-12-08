#!/bin/sh
#
# Start afl in NCPU + 1 threads to saturate CPU
#

NCPU=$(getconf _NPROCESSORS_ONLN)

DATADIR=data/
OUTDIR=result/
EXEC="./loaders @@"

for i in $(seq -w 1 $NCPU); do
	afl-fuzz -i "$DATADIR" -o "$OUTDIR" -S fuzzer$i $EXEC > /dev/null&
done

afl-fuzz -i "$DATADIR" -o "$OUTDIR" -M fuzzer00 $EXEC
