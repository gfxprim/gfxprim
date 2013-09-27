ifndef LIBNAME
$(error LIBNAME not defined, fix your library Makefile)
endif

ifdef SWIG
ifdef PYTHON_CONFIG

INCLUDES+=$(addprefix -I$(TOPDIR)/include/, $(INCLUDE))
#PY_SUFFIX=$(shell $(PYTHON_CONFIG) --extension-suffix)
PY_SUFFIX=.so

SWIG_SRC=$(LIBNAME).i
SWIG_C=$(LIBNAME)_wrap.c
SWIG_PY=c_$(LIBNAME).py
SWIG_LIB=_c_$(LIBNAME)$(PY_SUFFIX)

ALL+=$(SWIG_LIB) $(SWIG_PY)

# Empty rule to satisfy SWIG_PY
$(SWIG_PY): $(SWIG_C)
	@

$(SWIG_LIB): $(TOPDIR)/config.gen.mk

$(SWIG_C): $(SWIG_SRC)
ifdef VERBOSE
	$(SWIG) $(SWIGOPTS) -python $(INCLUDES) $<
else # VERBOSE
	@echo "SWIG $(LIBNAME)"
	@$(SWIG) $(SWIGOPTS) -python $(INCLUDES) $<
endif # VERBOSE

# All swig sources depend on common.i
$(SWIG_C): ../common.i

$(SWIG_LIB): $(SWIG_C)
ifdef VERBOSE
	$(CC) $< $(CFLAGS) -D_GNU_SOURCE=1 $(LDFLAGS) $(PYTHON_INCLUDE) --shared $(LDLIBS) $(LDLIBS_GP) -L$(TOPDIR)/build/ -o $@
else # VERBOSE
	@echo "LD   $@"
	@$(CC) $< $(CFLAGS) -D_GNU_SOURCE=1 $(LDFLAGS) $(PYTHON_INCLUDE) --shared $(LDLIBS) $(LDLIBS_GP) -L$(TOPDIR)/build/ -o $@
endif # VERBOSE

# Install python libraries into right places
INSTALL_FILES+=__init__.py $(SWIG_PY) $(SWIG_LIB)

include $(TOPDIR)/pyinst.mk

CLEAN+=$(SWIG_C) $(SWIG_PY) $(SWIG_LIB) *.pyc

endif # PYTHON_CONFIG
endif # SWIG
