SUBDIRS=$(dir $(wildcard */images.txt))

TARGETS=$(addsuffix images.txt,$(SUBDIRS))
IMGS=$(addsuffix *.png,$(SUBDIRS))

all: $(TARGETS)

$(TARGETS): ./regen.py
	@LD_LIBRARY_PATH=../../build/ PYTHONPATH=../../pylib/ ./regen.py

clean:
	rm -f $(TARGETS) $(IMGS)
