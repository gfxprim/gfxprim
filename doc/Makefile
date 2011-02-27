all: api.html index.html

index.html: index.txt
	asciidoc -a toc index.txt

api.html: api.txt context.txt loaders.txt
	asciidoc -a toc api.txt

api.pdf: api.txt
	asciidoc -b docbook api.txt
	xsltproc /usr/share/sgml/docbook/xsl-stylesheets/fo/docbook.xsl api.xml > api.fo
	fop api.fo -pdf api.pdf

clean:
	rm -f api.html index.html
	rm -f api.xml api.fo api.pdf
