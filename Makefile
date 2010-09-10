.DELTE_ON_ERROR:
.DEFAULT_GOAL:=test
.PHONY: all bin lib test clean

OUT:=out
SRC:=src
PROJ:=blossom
LIBBLOSSOM:=$(OUT)/lib$(PROJ)/lib$(PROJ).so
BLOSSOMTEST:=$(OUT)/$(PROJ)test/$(PROJ)test

LIB:=$(LIBBLOSSOM)
BIN:=$(BLOSSOMTEST)

CFLAGS+=-I$(SRC) -fvisibility=hidden -O2 -Wall

all: lib bin

bin: $(BIN)

lib: $(LIB)

test: all

$(OUT)/%: $(OUT)/%.o
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OUT)/libblossom/lib%.so: $(OUT)/libblossom/%.o
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LFLAGS)

$(OUT)/%.o: $(SRC)/%.c $(wildcard $(SRC)/*.h)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OUT)
