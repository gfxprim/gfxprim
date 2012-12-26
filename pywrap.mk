ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

SWIG_SRC=$(LIBNAME).i
SWIG_C=$(LIBNAME)_wrap.c
SWIG_PY=c_$(LIBNAME).py
SWIG_LIB=_c_$(LIBNAME).so

ifneq ($(SWIG),)

INCLUDES+=$(addprefix -I$(TOPDIR)/include/, $(INCLUDE))

ALL+=$(SWIG_LIB) $(SWIG_PY)

# Empty rule to satisfy SWIG_PY
$(SWIG_PY): $(SWIG_C)
	@

$(SWIG_C): $(SWIG_SRC)
ifdef VERBOSE
	$(SWIG) $(SWIGOPTS) -python $(INCLUDES) $<
else # VERBOSE
	@echo "SWIG $(LIBNAME)"
	@$(SWIG) $(SWIGOPTS) -python $(INCLUDES) $<
endif # VERBOSE

$(SWIG_LIB): $(SWIG_C)
ifdef VERBOSE
	$(CC) $< $(CFLAGS) -D_GNU_SOURCE=1 $(LDFLAGS) -I$(PYTHON_INCLUDE) --shared -lGP $(LDLIBS) -L$(TOPDIR)/build/ -o $@
else # VERBOSE
	@echo "LD  $@"
	@$(CC) $< $(CFLAGS) -D_GNU_SOURCE=1 $(LDFLAGS) -I$(PYTHON_INCLUDE) --shared -lGP $(LDLIBS) -L$(TOPDIR)/build/ -o $@
endif # VERBOSE

endif # ifneq ($(SWIG),)

CLEAN+=$(SWIG_C) $(SWIG_PY) $(SWIG_LIB) *.pyc
