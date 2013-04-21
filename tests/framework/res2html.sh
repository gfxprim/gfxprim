#!/bin/sh

#
# Quick hack to generate html from result directory
#

resdir=$1

die()
{
	echo ""
	echo "* ERROR * ERROR * ERROR * ERROR * ERROR * ERROR *"
	echo ""
	echo "$1"
	echo ""
	echo "* ERROR * ERROR * ERROR * ERROR * ERROR * ERROR *"
	echo ""
	exit 1
}

BINDIR=$(pwd)

echo "Running in resdir $resdir"

cd "$resdir" || die "Failed to cd into $resdir"

echo "<html>" > index.html
echo " <head>" >> index.html
echo " </head>" >> index.html
echo " <body>" >> index.html
echo "  <h1>Test Results</h1>" >> index.html
echo "  <table bgcolor=\"#99a\">" >> index.html
echo "   <tr bgcolor=\"#bbbbff\">" >> index.html
echo "    <td><b>Suite</b></td>" >> index.html
echo "    <td><b>Fail</b></td>" >> index.html
echo "    <td><b>Skip</b></td>" >> index.html
echo "    <td><b>All</b></td>" >> index.html
echo "    <td><b>Link</b></td>" >> index.html
echo "   </tr>" >> index.html

for i in *; do
	if ! [ -d "$i" ]; then
		continue
	fi

	echo "     <tr><td colspan=\"6\" bgcolor=\"#bbf\">&nbsp;$i</td></tr>" >> index.html

	for j in "$i/"*.json; do
		ts_name=$(basename "$j" ".json")
		echo "$j -> $i/$ts_name.html"
		$BINDIR/log2html.py -s "$i/$ts_name.html" "$j" >> index.html
		$BINDIR/log2html.py "$j" > "$i/$ts_name.html"
	done
done

echo "  </table>" >> index.html
echo " </body>" >> index.html
echo "</html>" >> index.html
