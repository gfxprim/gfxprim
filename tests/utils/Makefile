TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=vec.c matrix.c vec_str.c list.c htable.c utf.c json.c json_reader.c\
	 json_writer.c cfg.c trie.c avl_tree.c markup_plaintext.c markup_html.c\
	 markup_gfxprim.c markup_justify.c json_serdes.c path.c timer.c balloc.c\
	 heap.c cbuffer.c seek.c strconv.c

APPS=vec matrix vec_str list htable utf json json_reader json_writer\
     cfg trie avl_tree markup_plaintext markup_html markup_gfxprim\
     markup_justify json_serdes path timer balloc heap cbuffer seek\
     strconv

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
