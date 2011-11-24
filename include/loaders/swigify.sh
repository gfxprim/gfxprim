set -e

swig -python -Wall -I/usr/include/ gfxprim_loaders.swig
gcc -shared gfxprim_loaders_wrap.c -L ../../build/ -I /usr/include/python2.6/ -I.. \
  -lGP -lpng -ljpeg -lm -ldl -o ../../build/_gfxprim_loaders.so
mv gfxprim_loaders.py ../../build/
rm gfxprim_loaders_wrap.c

echo Swigified!
