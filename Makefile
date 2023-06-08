CFILES = $(wildcard src/*.c src/*/*.c)
OFILES = $(patsubst src/%.c, build/%.o, $(CFILES))
OBJECTS = src/main.c src/util.c src/lexer.c src/parser.c src/list.c src/string.c src/codegen.c src/hashmap.c

all: sil

build/%.o: src/%.c
	gcc -c -o $@ -Isrc `llvm-config --cflags` $<

sil: $(OFILES)
	gcc -o $@ `llvm-config --cflags --system-libs --ldflags --libs core` $(OFILES)

sil_old: $(OFILES)
	gcc $(OBJECTS) -o $@ `llvm-config --cflags --system-libs --ldflags --libs core`

clean:
	-rm ./sil build/*.o build/*/*.o