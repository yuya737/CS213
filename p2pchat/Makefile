CC := clang
CFLAGS := -g -Wall -Werror -Wno-unused-function -Wno-unused-variable

all: p2pchat

clean:
	rm -rf p2pchat p2pchat.dSYM

p2pchat: p2pchat.c ui.c ui.h
	$(CC) $(CFLAGS) -o p2pchat p2pchat.c ui.c -lform -lncurses -lpthread