.DELTE_ON_ERROR:
.DEFAULT_GOAL:=test
.PHONY: all bin lib test clean

OUT:=out
SRC:=src
PROJ:=blossom
LIBBLOSSOM:=$(OUT)/lib$(PROJ).so

CFLAGS+=-I$(SRC) -fvisibility=hidden -O2 -Wall

all: lib bin

bin:

lib: $(LIBBLOSSOM)

test: all

$(OUT)/lib%.so: $(OUT)/%.o
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LFLAGS)

$(OUT)/%.o: $(SRC)/%.c $(wildcard $SRC/*.h)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OUT)
