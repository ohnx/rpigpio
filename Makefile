CC ?= gcc
CFLAGS += -I. -Wall -Werror -O3
FILES = yhs.c rpigpio.c
OUTPUT = rpigpio

all: $(FILES) libgpio.o
	$(CC) $^ $(CFLAGS) -o $(OUTPUT)

# libgpio.c is ansi C
libgpio.o: libgpio.c
	$(CC) -I. -Wall -Werror -ansi -pedantic $^ -c -o $@

clean:
	rm -rf libgpio.o $(OUTPUT)
