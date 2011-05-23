#!/bin/sh

TOPDIR=..
LIBDIRS="core gfx text loaders filters backends input"

get_objects()
{
	for i in $LIBDIRS; do
		OBJECTS=`echo $TOPDIR/libs/$i/*.o`;

		if [ "$OBJECTS" != "$TOPDIR/libs/$i/*.o" ]; then
			echo "$OBJECTS"
		fi
	done
}

if [ "$1" = "libGP.a" ]; then
	OBJECTS=`get_objects`

	ar rcs libGP.a $OBJECTS
fi

if [ "$1" = "libGP.so" ]; then
	OBJECTS=`get_objects`

	gcc -fPIC -dPIC --shared -Wl,-soname -Wl,libGP.so.0 $OBJECTS -o libGP.so
fi
