CC ?= gcc
CFLAGS += -I. -Wall -Werror -O3
FILES = rpigpio.c
OUTPUT = rpigpio

# rest of the files are c99 (with GNU extensions)
$(OUTPUT): $(FILES) libgpio.o yhs.o
	$(CC) $^ $(CFLAGS) -std=gnu99 -o $(OUTPUT)

# libgpio.c is ansi C
libgpio.o: libgpio.c
	$(CC) -I. -Wall -Werror -ansi -pedantic -fPIC $^ -c -o $@
	$(CC) $@ -shared -o libgpio.so
	$(CC) -I. -Wall -Werror -ansi -pedantic $^ -c -o $@
	$(AR) -rv libgpio.a $@

# yhs.o is not ANSI C
yhs.o: yhs.c
	$(CC) $^ $(CFLAGS) -std=gnu99 -c -o $@

clean:
	rm -rf libgpio.o yhs.o $(OUTPUT)
