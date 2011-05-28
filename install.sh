#!/bin/sh

INSTALL_PREFIX="$1"

HEADER_LOC="$INSTALL_PREFIX/usr/include/"
LIB_LOC="$INSTALL_PREFIX/usr/lib/"

# Headers
echo "INSTALL headers"
install -m 775 -d "${HEADER_LOC}GP"
for i in `ls include/`; do
	if [ -d "include/$i" ]; then
		echo " $i"
		install -m 775 -d "${HEADER_LOC}GP/$i"
		install -m 664 "include/$i/"*.h "${HEADER_LOC}GP/$i"
	else
		install -m 664 "include/$i" "${HEADER_LOC}GP/$i"
	fi
done

# Library
echo "INSTALL libraries"
install -m 775 -d "$LIB_LOC" 
install -m 664 build/*.so build/*.so.0 build/*.a "$LIB_LOC"
