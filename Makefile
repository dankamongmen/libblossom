.DELTE_ON_ERROR:
.DEFAULT_GOAL:=test
.PHONY: all bin lib doc test clean install uninstall

VERSION=0.99.0

OUT:=out
SRC:=src
DOC:=doc
PROJ:=blossom
TAGS:=$(OUT)/tags
LIBBLOSSOM:=$(OUT)/lib$(PROJ)/lib$(PROJ).so
BLOSSOMTEST:=$(OUT)/$(PROJ)test/$(PROJ)test

# We intentionally use late binding for BIN_LFLAGS here, since realpath
# fails unless the file exists, and thus can't be evaluated until after
# the deps have been built...
BIN_LFLAGS=-Wl,-R$(realpath $(dir $(LIBBLOSSOM)))
BIN_LFLAGS+=-L$(dir $(LIBBLOSSOM)) -l$(PROJ)

LIB:=$(LIBBLOSSOM)
BIN:=$(BLOSSOMTEST)

CFLAGS+=-pthread -D_GNU_SOURCE -fpic -I$(SRC) -fvisibility=hidden -O2 -Wall -W -Werror
LFLAGS+=-Wl,-O,--default-symver,--enable-new-dtags,--as-needed,--warn-common
CTAGS?=$(shell (which ctags || echo ctags) 2> /dev/null)
XSLTPROC?=$(shell (which xsltproc || echo xsltproc) 2> /dev/null)

MAN3SRC:=$(wildcard $(DOC)/man/man3/*)
MAN3:=$(addprefix $(OUT)/,$(MAN3SRC:%.xml=%.3blossom))

all: $(TAGS) lib bin

bin: $(BIN)

doc: $(MAN3)

lib: $(LIB)

test: all
	for test in $(BIN) ; do ./$$test ; done

$(OUT)/%: $(OUT)/%.o $(LIB)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS) $(BIN_LFLAGS)

$(OUT)/libblossom/lib%.so: $(OUT)/libblossom/%.o
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LFLAGS)

$(OUT)/%.o: $(SRC)/%.c $(wildcard $(SRC)/lib$(PROJ)/*.h)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Should the network be inaccessible, and local copies are installed, try:
#DOC2MANXSL?=--nonet
$(OUT)/%.3blossom: %.xml
	@mkdir -p $(@D)
	$(XSLTPROC) --writesubtree $(@D) -o $@ $(DOC2MANXSL) $<

$(TAGS): $(wildcard $(SRC)/*/*.c) $(wildcard $(SRC)/*/*.h)
	@mkdir -p $(@D)
	$(CTAGS) -o $@ -R $(SRC)

clean:
	rm -rf $(OUT)

install: all

uninstall:
