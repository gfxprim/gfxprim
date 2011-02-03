all: api.html

api.html: api.txt
	asciidoc -a toc api.txt

api.pdf: api.txt
	asciidoc -b docbook api.txt
	xsltproc /usr/share/sgml/docbook/xsl-ns-stylesheets/fo/docbook.xsl api.xml > api.fo
	fop api.fo -pdf api.pdf
	

clean:
	rm -f api.html api.xml api.fo api.pdf
