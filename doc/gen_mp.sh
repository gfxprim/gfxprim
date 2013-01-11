#!/bin/sh
#
# Script to regenerate svgs from metapost
#
# The resulting svg images are stored in git so you need
# to run the script only after you changed the metapost
# source files.
#

SRC=coordinates

echo "$SRC.mp -> $SRC-*.pdf"
mptopdf "$SRC.mp"

for i in $SRC*.pdf; do
	SVG=`echo "$i" | sed s/.pdf/.svg/`
	echo "$i -> $SVG" 
	inkscape -l "$SVG" "$i"
	rm "$i"
done
