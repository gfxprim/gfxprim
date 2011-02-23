#!/bin/bash

echo "Generating pbm file"
./runtest.sh PBM_test &> /dev/null

echo -n "Testing pbm load/save ... "
./runtest.sh PBM_invert test.pbm test_inv.pbm &> /dev/null
./runtest.sh PBM_invert test_inv.pbm test_dinv.pbm &> /dev/null

if diff test.pbm test_dinv.pbm &> /dev/null; then
	echo "OK"
else
	echo "FAILED"
fi

echo "Generating pgm file"
./runtest.sh PGM_test &> /dev/null

echo -n "Testing pgm load/save ... "
./runtest.sh PGM_invert test.pgm test_inv.pgm &> /dev/null
./runtest.sh PGM_invert test_inv.pgm test_dinv.pgm &> /dev/null

if diff test.pgm test_dinv.pgm &> /dev/null; then
	echo "OK"
else
	echo "FAILED"
fi

rm -f test.pbm test_inv.pbm test_dinv.pbm
rm -f test.pgm test_inv.pgm test_dinv.pgm
