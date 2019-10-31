.DELETE_ON_ERROR:

LANG=C
LC_ALL=c

AM_CFLAGS=-O2 -Wall -W -Wshadow -Werror -Wextra -fvisibility=hidden \
	  	  	$(PTHREAD_CFLAGS)
AM_LDFLAGS=-Wl,-O2,--no-undefined-version,--enable-new-dtags,--as-needed,--warn-common
AM_CC=$(PTHREAD_CC)
AM_CPPFLAGS=-I $(SRCDIR)/src/lib

doc_DIR=doc
doc_DATA=
doc_DATA+=README.md

pkgconfigdir=${libdir}/pkgconfig
nodist_pkgconfig_DATA=libblossom.pc

LIBS+=$(PTHREAD_LIBS)

check_PROGRAMS=blossomtest

dist_man_MANS = blossom.3

dist_doc_DATA=blossom.xhtml

include_HEADERS=src/libblossom/blossom.h

lib_LTLIBRARIES=libblossom.la
libblossom_la_SOURCES=src/libblossom/blossom.c

blossomtest_SOURCES=src/blossomtest/blossomtest.c
blossomtest_LDADD=libblossom.la

check: blossomtest
	./$^

%.pc: %.pc.in configure.ac
	sed -e 's@#PREFIX#@${prefix}@' \
		-e 's@#VERSION#@${VERSION}@' \
		< $< > $@

%.3: $(doc_DIR)/man/man3/%.xml $(doc_DIR)/@PACKAGE_NAME@.ent
	$(XSLTPROC) --path $(doc_DIR) -o $@ --nonet /usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl $<

%.xhtml: $(doc_DIR)/man/man3/%.xml $(doc_DIR)/@PACKAGE_NAME@.ent
	$(XSLTPROC) --path $(doc_DIR) -o $@ --nonet /usr/share/xml/docbook/stylesheet/docbook-xsl/xhtml-1_1/docbook.xsl $<

$(doc_DIR)/@PACKAGE_NAME@.ent:
	@mkdir -p $(@D)
	echo -e '<!ENTITY dhfirstname "Nick">\n<!ENTITY dhsurname "Black">\n<!ENTITY dhemail "nick.black@sprezzatech.com">\n<!ENTITY dhusername "&dhfirstname; &dhsurname;">\n<!ENTITY dhrelease "$(VERSION)">' > "$@"

clean-local:
	rm -rf $(MANS) $(doc_DIR)/@PACKAGE_NAME@.ent $(nodist_pkgconfig_DATA)

uninstall-local:
	rmdir "$(DESTDIR)$(docdir)"
