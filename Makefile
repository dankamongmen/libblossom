.DELTE_ON_ERROR:
.DEFAULT_GOAL:=test
.PHONY: all bin lib test clean

VERSION=0.99.0

OUT:=out
SRC:=src
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
CTAGS?=ctags

all: $(TAGS) lib bin

bin: $(BIN)

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

$(TAGS): $(wildcard $(SRC)/*/*.c) $(wildcard $(SRC)/*/*.h)
	@mkdir -p $(@D)
	$(CTAGS) -o $@ -R $(SRC)

clean:
	rm -rf $(OUT)
