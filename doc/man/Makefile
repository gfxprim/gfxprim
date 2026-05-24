TOPDIR=../..
include $(TOPDIR)/pre.mk

all:
	./xml2man.py ../api-docs/xml/ .

clean:
	rm -f *.3

INSTALL_MAN3=$(wildcard ./*.3)

include $(TOPDIR)/install.mk
include $(TOPDIR)/post.mk
