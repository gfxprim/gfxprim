set -e

swig -python -Wall -I/usr/include/ gfxprim_loaders.swig
gcc -shared gfxprim_loaders_wrap.c -L ../../build/ -I /usr/include/python2.6/ -I.. \
  -fPIC -Wall -lGP -lpng -ljpeg -lm -ldl -o ../../build/_gfxprim_loaders_c.so
mv gfxprim_loaders_c.py ../../pylib/
rm gfxprim_loaders_wrap.c

echo Swigified!
