all: api.html

api.html: general.txt api.txt context.txt loaders.txt filters.txt \
          basic_types.txt drawing_api.txt
	asciidoc -a toc api.txt

api.pdf: api.txt
	asciidoc -b docbook api.txt
	xsltproc /usr/share/sgml/docbook/xsl-stylesheets/fo/docbook.xsl api.xml > api.fo
	fop api.fo -pdf api.pdf

clean:
	rm -f api.html
	rm -f api.xml api.fo api.pdf
