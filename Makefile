CFILES = $(wildcard src/*.c src/*/*.c)
OFILES = $(patsubst src/%.c, build/%.o, $(CFILES))
OBJECTS = src/main.c src/util.c src/lexer.c src/parser.c src/list.c src/string.c src/codegen.c src/hashmap.c src/token.c

all: sil

build/%.o: src/%.c
	gcc -c -o $@ -Isrc $<

sil: $(OFILES)
	gcc -o $@ $(OFILES)

sil_old: $(OFILES)
	gcc $(OBJECTS) -o $@

clean:
	-rm ./sil build/*.o
