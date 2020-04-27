CC := clang
CFLAGS := -g -Wall -Werror

all: shuffle every mywc

clean:
	rm -rf shuffle shuffle.dSYM every every.dSYM mywc mywc.dSYM

shuffle: shuffle.c
	$(CC) $(CFLAGS) -o shuffle shuffle.c

every: every.c
	$(CC) $(CFLAGS) -o every every.c

mywc: mywc.c
	$(CC) $(CFLAGS) -o mywc mywc.c
