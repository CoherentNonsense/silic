CFILES = $(wildcard src/*.c src/*/*.c)
OFILES = $(patsubst src/%.c, build/%.o, $(CFILES))

all: sil

build/%.o: src/%.c
	gcc -c -o $@ -Isrc $<

sil: $(OFILES)
	gcc -o $@ $(OFILES)

sil_old: $(OFILES)
	gcc $(OBJECTS) -o $@

clean:
	-rm ./sil build/*.o
