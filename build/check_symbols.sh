#!/bin/sh
#
# Purpose of this script is to check library exported symbols
#

SYMBOLFILE=libGP_symbols.txt

function grep_sym
{
	if objdump --syms "$1" |grep "$2" 2>&1 > /dev/null; then
		echo "SYM: $2 : $1"
	fi
}

function find_symbol
{
	find ../libs/ -name '*.o' | while read obj; do grep_sym "$obj" "$1"; done
}

objdump --dynamic-syms libGP.so | awk 'NR > 4 { print }' | awk '$3 != "*UND*"' | awk '{print $NF}'  > $SYMBOLFILE

#
# Remove a few known symbols
#
sed -i '/.init/d' $SYMBOLFILE
sed -i '/_init/d' $SYMBOLFILE
sed -i '/_end/d' $SYMBOLFILE
sed -i '/_fini/d' $SYMBOLFILE

for i in `cat libGP_symbols.txt`; do
	if ! grep "$i" libGP_expected_symbols.txt 2>&1 > /dev/null; then
		find_symbol "$i"
	fi
done

rm libGP_symbols.txt
