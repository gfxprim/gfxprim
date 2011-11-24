set -e

swig -python -Wall -I/usr/include/ gfxprim_core.swig
gcc -shared gfxprim_core_wrap.c -L ../../build/ -I /usr/include/python2.6/ -I.. \
  -fPIC -Wall -lGP -lpng -ljpeg -lm -ldl -o ../../build/_gfxprim_core_c.so
mv gfxprim_core_c.py ../../pylib/
rm gfxprim_core_wrap.c

echo Swigified!
