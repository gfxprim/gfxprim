all: api.html context.html

api.html: api.txt
	asciidoc -a toc api.txt

context.html: context.txt
	asciidoc -a context.txt

api.pdf: api.txt
	asciidoc -b docbook api.txt
	xsltproc /usr/share/sgml/docbook/xsl-ns-stylesheets/fo/docbook.xsl api.xml > api.fo
	fop api.fo -pdf api.pdf
	

clean:
	rm -f api.html api.xml api.fo api.pdf
