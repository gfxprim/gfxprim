#!/bin/sh
#
# Purpose of this script is to check library exported symbols
#

SYMBOLFILE=libGP_symbols.txt

function grep_sym
{
	if objdump --syms "$1" |grep "$2" 2>&1 > /dev/null; then
		echo -e "\t$1"
	fi
}

function find_symbol
{
	echo "SYM $1:"
	
	find ../libs/ -name '*.o' | while read obj; do grep_sym "$obj" "$1"; done
}

objdump --dynamic-syms libGP.so | awk 'NR > 4 { print }' | awk '$3 != "*UND*"' | awk '{print $NF}'  > $SYMBOLFILE

#
# Remove compiler and glibc symbols
#
sed -i '/^.init$/d' $SYMBOLFILE
sed -i '/^__.*$/d' $SYMBOLFILE
sed -i '/^_.*$/d' $SYMBOLFILE

for i in `cat libGP_symbols.txt`; do
	if ! grep "$i" syms/*.txt 2>&1 > /dev/null; then
		find_symbol "$i"
	fi
done

rm libGP_symbols.txt
