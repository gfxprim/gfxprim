#!/bin/sh
#
# Autogenerate examples page
#
OUT="$1"

get_names()
{
	LINKS=$(grep 'include:.*' $1)
	NAMES=$(echo $LINKS | sed 's/include:://g' | sed 's/\[\]//g')
	for i in $NAMES; do
		echo $(basename $i)
	done
}

table_header()
{
	echo ".List $1 examples" >> $OUT
	echo "[grid=\"rows\"]" >> $OUT
	echo '[options="autowidth,header",cols="<,<m"]' >> $OUT
	echo "|===========================" >> $OUT
	echo "| Example | Source filename(s)" >> $OUT
}

table_footer()
{
	echo "|===========================" >> $OUT
}

echo "List of examples" > $OUT
echo "----------------" >> $OUT
echo "Following pages contains a list of all code examples, the source " >> $OUT
echo "files could be found and are compiled in the +demos/c_simple/+ and" >> $OUT
echo "+demos/py_simple+ directories in the source tree." >> $OUT
echo >> $OUT

table_header "C"
for i in example_*.txt; do
	LINK=$(echo $i | sed s/.txt/.html/)
	DESC=$(head -n 1 $i)
	FNAMES=$(get_names "$i")
	if $(echo $i | grep -q '_py_'); then
		continue
	fi
	echo "| link:$LINK[$DESC] | $FNAMES" >> $OUT
done
table_footer

table_header "Python"
for i in example_py_*.txt; do
	LINK=$(echo $i | sed s/.txt/.html/)
	DESC=$(head -n 1 $i)
	FNAMES=$(get_names "$i")
	echo "| link:$LINK[$DESC] | $FNAMES" >> $OUT
done
table_footer
