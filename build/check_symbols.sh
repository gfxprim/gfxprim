#!/bin/sh
#
# Purpose of this script is to check library exported symbols
#

SYMTMPFILE=symbols.txt

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

function get_symbols
{
	objdump --dynamic-syms "$1" | awk 'NR > 4 { print }' | awk '$3 != "*UND*"' | awk '{print $NF}'  > "$2"

	#
	# Remove compiler and glibc symbols
	#
	sed -i '/^.init$/d' "$2"
	sed -i '/^__.*$/d' "$2"
	sed -i '/^_.*$/d' "$2"
}

function check_symbols
{
	local symfile=$1
	shift

	for i in `cat $symfile`; do
		if ! grep "^$i$" $@ 2>&1 > /dev/null; then
			find_symbol "$i"
		fi
	done
}

function do_check
{
	get_symbols "$1" $SYMTMPFILE
	shift
	check_symbols $SYMTMPFILE "$@"
	rm $SYMTMPFILE
}

do_check libGP.so syms/Core_symbols.txt syms/Input_symbols.txt \
                  syms/Filters_symbols.txt syms/GFX_symbols.txt \
                  syms/Text_symbols.txt

do_check libGP_backends.so syms/Backend_symbols.txt

do_check libGP_grabbers.so syms/Grabbers_symbols.txt

do_check libGP_loaders.so syms/Loaders_symbols.txt
