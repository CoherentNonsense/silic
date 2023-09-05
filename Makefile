CFILES = $(wildcard src/*.c src/*/*.c)
OFILES = $(patsubst src/%.c, build/%.o, $(CFILES))

all: silic

build/%.o: src/%.c
	gcc -c -o $@ -Isrc $<

silic: $(OFILES)
	gcc -o $@ $(OFILES)

clean:
	-rm ./silic build/*.o
