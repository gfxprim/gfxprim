set -e

swig -python -Wall -I/usr/include/ gfxprim_core.swig
gcc -shared gfxprim_core_wrap.c -L ../../build/ -I /usr/include/python2.6/ -I.. \
  -fPIC -lGP -lpng -ljpeg -lm -ldl -o ../../build/_gfxprim_core.so
mv gfxprim_core.py ../../build/
rm gfxprim_core_wrap.c

echo Swigified!
