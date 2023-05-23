OBJECTS = src/main.c src/util.c src/lexer.c src/parser.c src/list.c src/string.c src/codegen.c src/hashmap.c

sil: $(OBJECTS)
	gcc $(OBJECTS) -o $@ `llvm-config --cflags --system-libs --ldflags --libs core`

clean:
	-rm -f sil
