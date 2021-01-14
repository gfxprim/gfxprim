TOPDIR=..
include $(TOPDIR)/config.mk

SOURCES=index.html about.txt pixmap.txt loaders.txt filters.txt \
	basic_types.txt gfx.txt backends.txt gamma.txt grabbers.txt \
	environment_variables.txt debug.txt core.txt input.txt \
	gen.txt pixels.txt coordinate_system.txt coding_style.txt \
	get_put_pixel.txt blits.txt progress_callback.txt text.txt \
	event_queue.txt compilation.txt filters_resize.txt \
	filters_dithering.txt filters_python.txt spiv.txt core_common.txt \
	convert.txt news_1_0_0-rc1.txt loaders_io.txt signatures.txt widgets.txt \
	packages.txt

SOURCES+=core_python.txt gfx_python.txt loaders_python.txt backends_python.txt

SOURCES+=widget_grid.txt widget_table.txt widget_button.txt widget_label.txt widgets_tattr.txt\
	 widget_checkbox.txt

EXAMPLE_SOURCES=$(wildcard example_*.txt)

ASCIIDOC_PARAMS=--conf-file asciidoc.conf

#
# Names of generated images for cleanup
#
GENIMAGES=discrete_linear_1D_convolution_alg1 discrete_linear_1D_convolution_alg2 \
	  discrete_linear_convolution discrete_linear_convolution_alg1 \
	  discrete_linear_convolution_alg2 laplacian_edge_sharpening laplacian_kernel

PAGES=$(subst .txt,.html,$(SOURCES))
PAGES+=$(subst .txt,.html,$(EXAMPLE_SOURCES))
PAGES+=examples.html

.PHONY: toolcheck clean

all: toolcheck examples.html $(PAGES)

$(PAGES): asciidoc.conf

#
# Dependencies
#
IMG_INCLUDES=images/blur/images.txt images/median/images.txt\
             images/edge_sharpening/images.txt images/gaussian_noise/images.txt

filters_python.html: $(IMG_INCLUDES)
filters.html: $(IMG_INCLUDES)

#
# Check if we have all the needed tools
#
toolcheck:
	@if [ -z `which asciidoc 2>/dev/null` ] ; then \
		echo "ERROR: Could not find 'asciidoc'" ; exit 127 ; fi
	@if [ -z `which source-highlight 2>/dev/null` ] ; then \
		echo "ERROR: Could not find 'source-highlight'" ; exit 127 ; fi
	@if [ ! -d "/usr/share/graphviz" ] ; then \
		echo "ERROR: Could not find 'graphviz'" ; exit 127 ; fi
	@if [ -z `which latex 2>/dev/null` ] ; then \
		echo "error: could not find 'latex'" ; exit 127 ; fi
	@if [ -z `which dvipng 2>/dev/null` ] ; then \
		echo "error: could not find 'dvipng'" ; exit 127 ; fi

#
# Create page for each part of API description
#
$(PAGES): %.html: %.txt
	asciidoc $(ASCIIDOC_PARAMS) $<

#
# Autogenerate examples page
#
examples.txt: $(EXAMPLE_SOURCES) gen_examples_page.sh
	./gen_examples_page.sh examples.txt

#
# Clean up generated images
#
CLEAN+=$(patsubst %,%.md5,$(GENIMAGES))
CLEAN+=$(patsubst %,%.png,$(GENIMAGES))
CLEAN+=examples.txt

#
# Clean up generated pages
#
CLEAN+=$(PAGES)

clean:
	rm -f $(CLEAN)

INSTALL_DOC=$(PAGES) *.png *.svg asciidoc.css images/*/*.png
install: $(INSTALL_DOC)

include $(TOPDIR)/install.mk
