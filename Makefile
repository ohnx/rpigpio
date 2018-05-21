CC ?= gcc
CFLAGS += -I. -Wall -Werror -O3
FILES = yhs.c rpigpio.c
OUTPUT = rpigpio

# rest of the files are c99 (with GNU extensions)
$(OUTPUT): $(FILES) libgpio.o
	$(CC) $^ $(CFLAGS) -std=gnu99 -o $(OUTPUT)

# libgpio.c is ansi C
libgpio.o: libgpio.c
	$(CC) -I. -Wall -Werror -ansi -pedantic -fPIC $^ -c -o $@
	$(CC) $@ -shared -o libgpio.so
	$(CC) -I. -Wall -Werror -ansi -pedantic $^ -c -o $@
	$(AR) -rv libgpio.a $@

clean:
	rm -rf libgpio.o $(OUTPUT)
