TOPDIR=..

#
# Add .pyc files to CLEAN list
#
CLEAN+=$(patsubst %.py, %.pyc, ${PYTHON_FILES})

include $(TOPDIR)/include.mk

all:
	ls $(PYLIBSDIR)
	echo $(PYTHON_FILES)
	echo $(CLEAN)
	@true
