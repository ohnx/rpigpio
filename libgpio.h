/* ohnx was here (2018) */
#ifndef __LIBGPIO_INC
#define __LIBGPIO_INC

/* snprintf() */
#include <stdio.h>
/* ssize_t */
#include <sys/types.h>
/* open(), write(), close(), access() */
#include <unistd.h>

/* No error */
#define LIBGPIO_RET_OK                  0
/* Invalid input */
#define LIBGPIO_RET_INVALIDIN           1
/* Failed to communicate with the kernel */
#define LIBGPIO_RET_KERNREQ_FAIL        2
/* Failed to write GPIO direction */
#define LIBGPIO_RET_SETGPIODIR_FAIL     4
/* Failed to write GPIO value */
#define LIBGPIO_RET_SETGPIO_FAIL        8
/* Failed to read GPIO value */
#define LIBGPIO_RET_READGPIO_FAIL       16

/* unsigned 8-bit integer */
typedef unsigned char uint8_t;

/* function prototypes */
int libgpio_write(uint8_t gpio, uint8_t value, uint8_t keepexported);
int libgpio_read(uint8_t gpio, uint8_t *value, uint8_t keepexported);

#endif /* __LIBGPIO_INC */
