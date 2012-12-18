SOURCES=general.txt context.txt loaders.txt filters.txt basic_types.txt \
        drawing_api.txt backends.txt gamma.txt grabbers.txt \
	environment_variables.txt debug.txt core.txt api.txt input.txt

EXAMPLE_SOURCES=$(wildcard example_*.txt)

#
# Names of generated images for cleanup
#
GENIMAGES=discrete_linear_1D_convolution_alg1 discrete_linear_1D_convolution_alg2 \
	  discrete_linear_convolution discrete_linear_convolution_alg1 \
	  discrete_linear_convolution_alg2 laplacian_edge_sharpening laplacian_kernel

PAGES=$(subst .txt,.html,$(SOURCES))
PAGES+=$(subst .txt,.html,$(EXAMPLE_SOURCES))

all: examples.html $(PAGES) api.html

#
# Create page for each part of API description
#
$(PAGES): %.html: %.txt
	asciidoc $<

examples.html: examples.txt ../demos/c_simple/*.c ../demos/py_simple/*.py
	asciidoc -a toc examples.txt

#
# Clean up generated images
#
CLEAN+=$(patsubst %,%.md5,$(GENIMAGES))
CLEAN+=$(patsubst %,%.png,$(GENIMAGES))

#
# Clean up generated pages
#
CLEAN+=$(PAGES)

clean:
	rm -f $(CLEAN)
	rm -f api_links.html api.html examples.html
	rm -f api.xml api.fo api.pdf
