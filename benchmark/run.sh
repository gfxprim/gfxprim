#!/bin/bash

OUTFILE=results.html
SUBDIRS="line triangle"

echo -ne "<html>\n <body>\n  <h1>Test results</h1>\n   " > "$OUTFILE"
echo "<b>" >> "$OUTFILE"
uname -o -p -m >> "$OUTFILE"
cat /proc/cpuinfo |grep bogomips >> "$OUTFILE"
echo "</b><br>" >> "$OUTFILE"
date >> "$OUTFILE"
echo "  <table>" >> "$OUTFILE"

for i in $SUBDIRS; do
	cd $i && rm -f results.html && ./run.sh
	cat results.html >> ../"$OUTFILE"
	rm results.html
	cd ..
done

echo -e "  </table>\n </body>\n</html>" >> "$OUTFILE"
