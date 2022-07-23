#!/bin/sh

resdir=$1
retval=0

die()
{
	echo "$1"
	exit 1
}

BINDIR=$(pwd)

cd "$resdir" || die "Failed to cd into $resdir"

for i in *; do
	if ! [ -d "$i" ]; then
		continue
	fi

	for j in "$i/"*.json; do
		$BINDIR/log2html.py -e "$j"

		if [ $? -ne 0 ]; then
			retval=1
		fi
	done
done
exit $retval
