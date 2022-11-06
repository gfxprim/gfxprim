TOPDIR=../..
include $(TOPDIR)/pre.mk

CSOURCES=vec.c matrix.c vec_str.c list.c htable.c utf.c json.c json_reader.c json_writer.c\
	 cfg.c trie.c avl_tree.c

APPS=vec matrix vec_str list htable utf json json_reader json_writer\
     cfg trie avl_tree

include ../tests.mk

include $(TOPDIR)/gen.mk
include $(TOPDIR)/app.mk
include $(TOPDIR)/post.mk
