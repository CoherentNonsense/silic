CFILES = $(wildcard src/*.c src/*/*.c)
OFILES = $(patsubst src/%.c, build/%.o, $(CFILES))

all: silic

build/%.o: src/%.c
	gcc -std=c11 -g -c -o $@ -Isrc $< -Wall -Wextra -pedantic -lchn

silic: $(OFILES)
	gcc -std=c11 -o $@ $(OFILES) -lchn

clean:
	-rm ./silic build/*.o
