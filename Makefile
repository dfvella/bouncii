CC = gcc
FLAGS = -Wconversion -Wall -Werror -Wextra -pedantic -lncurses -lm

all: bouncii bouncii_debug

bouncii: bouncii.c
	$(CC) $(FLAGS) -O3 $^ -o $@

bouncii_debug: bouncii.c
	$(CC) $(FLAGS) -g $^ -o $@

clean:
	rm -f bouncii bouncii_debug
