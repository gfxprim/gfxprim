all: print libs

print:
ifndef VERBOSE
	@echo "GEN (linker) libGP.a libGP.so"
endif

libs: print libGP.a libGP.so

#
# Do NOT remove this
#
.PHONY: libGP.a libGP.so

clean:
ifdef VERBOSE
	rm -f libGP.a libGP.so
else
	@echo "RM  libGP.a libGP.so"
	@rm -f libGP.a libGP.so
endif

libGP.a:
	@. ./liblock.sh; spinlock .
ifdef VERBOSE
	./link.sh libGP.a
else
	@echo "AR  libGP.a"
	@./link.sh libGP.a
endif
	@. ./liblock.sh; spinunlock .

libGP.so:
	@. ./liblock.sh; spinlock .
ifdef VERBOSE
	./link.sh libGP.so
else
	@echo "LD  libGP.so"
	@./link.sh libGP.so
endif
	@. ./liblock.sh; spinunlock .
