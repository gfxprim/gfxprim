#!/bin/sh

INSTALL_PREFIX="$1"

if test -z "$INSTALL_PREFIX"; then
	INSTALL_PREFIX="/usr/"
fi

HEADER_LOC="$INSTALL_PREFIX/include/"
LIB_LOC="$INSTALL_PREFIX/lib/"
BIN_LOC="$INSTALL_PREFIX/bin/"

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

# Binaries
echo "INSTALL binaries"
install -m 755 gfxprim-config "$BIN_LOC"
