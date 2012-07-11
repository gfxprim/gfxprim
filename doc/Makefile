SOURCES=general.txt context.txt loaders.txt filters.txt basic_types.txt \
        drawing_api.txt backends.txt gamma.txt grabbers.txt \
	environment_variables.txt debug.txt

PAGES=$(subst .txt,.html,$(SOURCES))


all: api.html examples.html $(PAGES) api_links.html

#
# Create all in one API page
#
api.html: $(SOURCES) api.txt
	asciidoc -a toc api.txt

#
# Create page for each part of API description
#
$(PAGES): %.html: %.txt
	asciidoc $<

api_links.html: api_links.txt
	asciidoc $<

examples.html: examples.txt ../demos/c_simple/*.c ../demos/py_simple/*.py
	asciidoc -a toc examples.txt

api.pdf: api.txt
	asciidoc -b docbook api.txt
	xsltproc /usr/share/sgml/docbook/xsl-stylesheets/fo/docbook.xsl api.xml > api.fo
	fop api.fo -pdf api.pdf

clean:
	rm -f api.html examples.html
	rm -f api.xml api.fo api.pdf
