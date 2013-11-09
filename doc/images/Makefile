SUBDIRS=$(dir $(wildcard */images.txt))

TARGETS=$(addsuffix images.txt,$(SUBDIRS))
IMGS=$(addsuffix *.png,$(SUBDIRS))

all: $(TARGETS)

$(TARGETS): ./regen.py
	@./regen.py

clean:
	rm -f $(TARGETS) $(IMGS)
