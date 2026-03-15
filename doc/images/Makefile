SUBDIRS=$(dir $(wildcard */images.txt))

TARGETS=$(addsuffix images.txt,$(SUBDIRS)) trigger
IMGS=$(addsuffix *.png,$(SUBDIRS))

all: $(TARGETS)

.PHONY: trigger

$(TARGETS): ./regen.py
	@LD_LIBRARY_PATH=../../build/ PYTHONPATH=../../pylib/ ./regen.py

clean:
	rm -f $(TARGETS) $(IMGS)
