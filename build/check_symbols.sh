#!/bin/sh
#
# Purpose of this script is to check library exported symbols
#

FOUND=""
WARN="WARNING : WARNING : WARNING : WARNING : WARNING : WARNING : WARNING : WARNING"

SYMTMPFILE=symbols.txt

grep_sym()
{
	if objdump --syms "$1" |grep "$2" 2>&1 > /dev/null; then
		echo -e "\t$1"
	fi
}

find_symbol()
{
	echo "SYM $1:"

	find ../libs/ -name '*.o' | while read obj; do grep_sym "$obj" "$1"; done
}

get_symbols()
{
	objdump --dynamic-syms "$1" | awk 'NR > 4 { print }' | awk '$3 != "*UND*"' | awk '{print $NF}'  > "$2"

	#
	# Remove compiler and glibc symbols
	#
	sed -i '/^.init$/d' "$2"
	sed -i '/^__.*$/d' "$2"
	sed -i '/^_.*$/d' "$2"
}

check_symbols()
{
	local symfile=$1
	shift

	for i in `cat $symfile`; do
		if ! grep "^$i$" $@ 2>&1 > /dev/null; then
			if [ -z "$FOUND" ]; then
				echo "$WARN"
				echo
				echo "Following new API symbols found:"
				echo
			fi
			find_symbol "$i"
			FOUND="yes"
		fi
	done
}

do_check()
{
	get_symbols "$1" $SYMTMPFILE
	shift
	check_symbols $SYMTMPFILE "$@"
	rm $SYMTMPFILE
}

do_check libgfxprim.so syms/core_symbols.txt syms/input_symbols.txt \
                       syms/filters_symbols.txt syms/gfx_symbols.txt \
                       syms/text_symbols.txt syms/utils_symbols.txt

do_check libgfxprim-backends.so syms/backend_symbols.txt

do_check libgfxprim-grabbers.so syms/grabbers_symbols.txt

do_check libgfxprim-loaders.so syms/loaders_symbols.txt

if [ -n "$FOUND" ]; then
	echo
	echo "Set them static or update lists of exported functions in syms/foo_symbols.txt"
	echo
	echo "$WARN"
else
	echo "No unexpected symbols found :)"
fi
