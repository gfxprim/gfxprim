#!/bin/sh

INSTALL_PREFIX="$1"

if test -z "$INSTALL_PREFIX"; then
	INSTALL_PREFIX="/usr"
fi

HEADER_LOC="$INSTALL_PREFIX/include/"
if [ -d $INSTALL_PREFIX/lib64 ]; then
	LIB_LOC="$INSTALL_PREFIX/lib64/"
else
	LIB_LOC="$INSTALL_PREFIX/lib/"
fi

BIN_LOC="$INSTALL_PREFIX/bin/"

# Headers
echo "INSTALL headers ($HEADER_LOC)"
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
echo "INSTALL libraries ($LIB_LOC)"
install -m 775 -d "$LIB_LOC" 

for i in build/*.so build/*.so.* build/*.a; do
	if [ -L "$i" ]; then
	        TARGET=`basename "$i"`
		SOURCE=`readlink "$i"`
		(cd "$LIB_LOC" && rm -f "$TARGET" && ln -s "$LIB_LOC$SOURCE" "$TARGET")
	else
		install "$i" "$LIB_LOC"
	fi
done

# Binaries
echo "INSTALL binaries"
install -m 755 gfxprim-config "$BIN_LOC"
install -m 755 demos/spiv/spiv "$BIN_LOC"
