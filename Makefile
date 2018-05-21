CC ?= gcc
CFLAGS += -I. -Wall -Werror -O0 -g
OUTPUT = rpigpio
ifdef TARGET
SUFFIX = -$(TARGET)
else
SUFFIX = 
endif


# rest of the files are c99 (with GNU extensions)
$(OUTPUT)$(SUFFIX): rpigpio.c libgpio$(SUFFIX).o yhs$(SUFFIX).o
	$(CC) $^ $(CFLAGS) -std=gnu99 -o $(OUTPUT)$(SUFFIX)

# libgpio.c is ansi C
libgpio$(SUFFIX).o: libgpio.c
	$(CC) -I. -Wall -Werror -ansi -pedantic -fPIC $^ -c -o $@
	$(CC) $@ -shared -o libgpio$(SUFFIX).so
	$(CC) -I. -Wall -Werror -ansi -pedantic $^ -c -o $@
	$(AR) -rv libgpio$(SUFFIX).a $@

# yhs.o is not ANSI C
yhs$(SUFFIX).o: yhs.c
	$(CC) $^ $(CFLAGS) -std=gnu99 -c -o $@

clean:
	rm -rf libgpio.o libgpio-*.o yhs.o yhs-*.o $(OUTPUT) $(OUTPUT)-* *.a *.so
