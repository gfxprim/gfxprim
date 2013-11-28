#!/bin/sh

PREFIX="$1"
LIBDIR="$2"
BINDIR="$3"
INCLUDEDIR="$4"

HEADER_LOC="${DESTDIR}$PREFIX/$INCLUDEDIR"
LIB_LOC="${DESTDIR}$PREFIX/$LIBDIR"
BIN_LOC="${DESTDIR}$PREFIX/$BINDIR"

# Headers
echo "INSTALL headers ($HEADER_LOC)"
install -m 775 -d "${HEADER_LOC}/GP"
for i in `ls include/`; do
	if [ -d "include/$i" ]; then
		echo " $i"
		install -m 775 -d "${HEADER_LOC}/GP/$i"
		install -m 664 "include/$i/"*.h "${HEADER_LOC}/GP/$i"
	else
		if [ "$i" != "Makefile" ]; then
			install -m 664 "include/$i" "${HEADER_LOC}/GP/$i"
		fi
	fi
done

# Library
echo "INSTALL libraries ($LIB_LOC)"
install -m 775 -d "$LIB_LOC"

for i in build/*.so build/*.so.* build/*.a; do
	if [ -L "$i" ]; then
	        TARGET=`basename "$i"`
		SOURCE=`readlink "$i"`
		(cd "$LIB_LOC" && rm -f "$TARGET" && ln -s "$SOURCE" "$TARGET")
	else
		install "$i" "$LIB_LOC"
	fi
done

# Binaries
echo "INSTALL binaries"
install -m 775 -d "$BIN_LOC"
install -m 755 gfxprim-config "$BIN_LOC"
