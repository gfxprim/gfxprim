SUBDIRS=blur median edge_sharpening gaussian_noise

TARGETS=$(addsuffix /images.txt,$(SUBDIRS))
IMGS=$(addsuffix /*.png,$(SUBDIRS))

all: $(TARGETS)

$(TARGETS): ./regen.py
	@./regen.py

clean:
	rm -f $(TARGETS) $(IMGS)
