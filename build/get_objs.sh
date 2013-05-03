#!/bin/sh

TOPDIR=..
LIBDIRS="core gfx text filters input"

for i in $LIBDIRS; do
	OBJECTS=`echo $TOPDIR/libs/$i/*.o`;

	if [ "$OBJECTS" != "$TOPDIR/libs/$i/*.o" ]; then
		echo "$OBJECTS"
	fi
done
