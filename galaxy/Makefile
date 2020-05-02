CC := clang
CFLAGS := -g $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lm

all: galaxy

clean:
	rm -f galaxy

galaxy: main.c gui.c gui.h
	$(CC) $(CFLAGS) -o galaxy main.c gui.c $(LDFLAGS)
