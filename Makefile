CC ?= gcc
CFLAGS = -I. -Wall -Werror -ansi -pedantic
FILES = libgpio.c rpigpio.c
OUTPUT = rpigpio

all: $(FILES) yhs.o
	$(CC) $^ $(CFLAGS) -o $(OUTPUT)

# needed because this file uses a painful number of non-ansi things like //
yhs.o: yhs.c
	$(CC) -I. -Wall -Werror $^ -c -o $@

clean:
	rm -rf yhs.o $(OUTPUT)
