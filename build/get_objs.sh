#!/bin/sh

TOPDIR=..
LIBDIRS="core gfx text filters input utils $1"

for i in $LIBDIRS; do
	OBJECTS=`echo $TOPDIR/libs/$i/*.o`;

	if [ "$OBJECTS" != "$TOPDIR/libs/$i/*.o" ]; then
		echo "$OBJECTS"
	fi

	OS_OBJECTS=`echo $TOPDIR/libs/$i/$1/*.o`;

	if [ "$OS_OBJECTS" != "$TOPDIR/libs/$i/$1/*.o" ]; then
		echo "$OS_OBJECTS"
	fi
done
