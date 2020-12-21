#!/bin/sh

PREFIX="$1"
LIBDIR="$2"
BINDIR="$3"
INCLUDEDIR="$4"

HEADER_LOC="${DESTDIR}/$INCLUDEDIR"
LIB_LOC="${DESTDIR}/$LIBDIR"
BIN_LOC="${DESTDIR}/$BINDIR"

# Headers
echo "INSTALL headers ($HEADER_LOC)"
install -m 775 -d "${HEADER_LOC}/gfxprim"
for i in `ls include/`; do
	if [ -d "include/$i" ]; then
		echo " $i"
		install -m 775 -d "${HEADER_LOC}/gfxprim/$i"
		install -m 664 "include/$i/"*.h "${HEADER_LOC}/gfxprim/$i"
	else
		if [ "$i" != "Makefile" ]; then
			install -m 664 "include/$i" "${HEADER_LOC}/gfxprim/$i"
		fi
	fi
done

# Library
echo "INSTALL libraries ($LIB_LOC)"
install -m 775 -d "$LIB_LOC"

for i in build/*.so build/*.so.*; do
	if [ -L "$i" ]; then
	        TARGET=`basename "$i"`
		SOURCE=`readlink "$i"`
		(cd "$LIB_LOC" && rm -f "$TARGET" && ln -s "$SOURCE" "$TARGET")
	else
		install "$i" "$LIB_LOC"
	fi
done

for i in build/*.a; do
	install -m 644 "$i" "$LIB_LOC"
done

# Binaries
echo "INSTALL binaries"
install -m 775 -d "$BIN_LOC"
install -m 755 gfxprim-config "$BIN_LOC"
