TOPDIR=../..
include $(TOPDIR)/pre.mk

APPS=loaders

CSOURCES=$(shell echo *.c)

# Link the binary statically
LIBS=$(shell $(TOPDIR)/gfxprim-config --libs-loaders --libs)
RLIBS=$(subst -lgfxprim-loaders,$(TOPDIR)/build/libgfxprim-loaders.a,$(LIBS))
LDLIBS=$(subst -lgfxprim,$(TOPDIR)/build/libgfxprim.a,$(RLIBS))

afl: loaders
	./run_afl.sh

include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
