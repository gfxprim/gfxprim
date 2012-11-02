#!/bin/sh

DIRS="core loaders gfx"

for i in $DIRS; do
	cd $i
	./runtest.sh $@
	cd ..
done
