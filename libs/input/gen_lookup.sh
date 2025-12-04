#!/bin/sh
#
# Generate lookup table for key_name to value translation
#

KEYS=$(awk '/\tGP_KEY_/{print $1}' ../../include/input/gp_event_keys.h | sort)
BTNS=$(awk '/\tGP_BTN_/{print $1}' ../../include/input/gp_event_keys.h | sort)

echo "// SPDX-License-Identifier: LGPL-2.1-or-later"
echo "/*"
echo " * Generated file, do not edit!"
echo " */"
echo
echo "#ifndef INPUT_KEYS_H"
echo "#define INPUT_KEYS_H"
echo
echo "#include <input/gp_event_keys.h>"
echo
echo "struct name_val {"
echo "	const char *name;"
echo "	int val;"
echo "};"
echo
echo "static struct name_val keys[] = {"

for i in $BTNS $KEYS; do
	name=$(echo "$i" | awk '{split($0, words, /[^a-zA-Z0-9]+/);
                                 for (i=2; i<=length(words); i++) {
                                  res = res toupper(substr(words[i],1,1)) tolower(substr(words[i],2));
                                 }
	                         print res}')
	echo "	{.name = \"$name\", .val = $i},"
done
echo "};"
echo
echo "static const char *key_names[] = {"

for i in $BTNS; do
	name=$(echo "$i" | awk '{split($0, words, /[^a-zA-Z0-9]+/);
                                 for (i=2; i<=length(words); i++) {
                                  res = res toupper(substr(words[i],1,1)) tolower(substr(words[i],2));
                                }
	                        print res}')
	echo "	[$i] = \"$name\","
done

for i in $KEYS; do
	name=$(echo "$i" | awk '{split($0, words, /[^a-zA-Z0-9]+/);
                                 for (i=3; i<=length(words); i++) {
                                  res = res toupper(substr(words[i],1,1)) tolower(substr(words[i],2));
                                }
	                        print res}')
	echo "	[$i] = \"$name\","
done

echo "};"
echo
echo "#endif /* INPUT_KEYS_H */"
