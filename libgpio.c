/* ohnx was here (2018) */
#include "libgpio.h"

#define _LIBGPIO_MODE_EXPORT         248
#define _LIBGPIO_MODE_UNEXPORT       249

/**
 * @fn static int _libgpio_rwfile(uint8_t gpio, uint8_t mode)
 * @brief Requests or surrenders access to a GPIO pin
 * @param gpio  The GPIO pin number to request access to
 * @param mode  The mode (_LIBGPIO_MODE_EXPORT or _LIBGPIO_MODE_UNEXPORT)
 * @return LIBGPIO_RET_OK if no errors occurred; appropriate error code otherwise
 * 
 * This function either asks the kernel for permission to read and write to the
 * specific GPIO pin (_LIBGPIO_MODE_EXPORT) or releases access
 * (_LIBGPIO_MODE_UNEXPORT).
 */
static int _libgpio_rwfile(uint8_t gpio, uint8_t mode) {
    static char buf[4];
    const char *f;
	ssize_t n;
	int fd;

    /* map input to correct files */
    switch (mode) {
    case _LIBGPIO_MODE_EXPORT: f = "/sys/class/gpio/export"; break;
    case _LIBGPIO_MODE_UNEXPORT: f = "/sys/class/gpio/unexport"; break;
    default: return LIBGPIO_RET_INVALIDIN;
    }

    /* open the file for writing */
	fd = open(f, O_WRONLY);
	/* busy??? not sure why this would fail */
	if (fd == -1) return LIBGPIO_RET_KERNREQ_FAIL;

    /* write out the gpio number */
	n = sprintf(buf, "%u", gpio);
	/* note: sprintf returns the length of string written without null; */
	/* the system API is expecting the string without a null terminator too. */
	n = write(fd, buf, n);
	/* nonexistent GPIO pin??? not sure why this would fail */
    if (n == -1) return LIBGPIO_RET_KERNREQ_FAIL;

    /* done */
	close(fd);
	return LIBGPIO_RET_OK;
}

#define _LIBGPIO_MODE_IN        250
#define _LIBGPIO_MODE_OUT       251
#define _LIBGPIO_MODE_LOW       252
#define _LIBGPIO_MODE_HIGH      253

/**
 * @fn static int _libgpio_setdir(uint8_t gpio, uint8_t dir)
 * @brief Sets the read or write mode of the GPIO
 * @param gpio  The GPIO pin number to request access to
 * @param dir   The direction of the pin (in or out)
 * @return LIBGPIO_RET_OK if no errors occurred; appropriate error code otherwise
 * 
 * This function sets the appropriate direction for a GPIO pin.
 */
static int _libgpio_setdir(uint8_t gpio, uint8_t dir) {
    static char buf[34];
    const char *f = "inoutlowhigh";
	ssize_t n, nf;
    int fd;

    /* map input to correct string */
    switch (dir) {
    case _LIBGPIO_MODE_IN: nf = 2; break;
    case _LIBGPIO_MODE_OUT: f += 2; nf = 3; break;
    case _LIBGPIO_MODE_LOW: f += 5; nf = 3; break;
    case _LIBGPIO_MODE_HIGH: f += 8; nf = 4; break;
    }

    /* open the file for writing */
    n = sprintf(buf, "/sys/class/gpio/gpio%u/direction", gpio);
    fd = open(buf, O_WRONLY);
    /* attribute does not exist, kernel doesn't support changing direction, no permission */
    if (fd == -1) return LIBGPIO_RET_SETGPIODIR_FAIL;

    /* write the appropriate message */
    n = write(fd, f, nf);
    /* can't change direction??? not sure why this would fail */
    if (n == -1) return LIBGPIO_RET_KERNREQ_FAIL;

    /* done */
    close(fd);
    return LIBGPIO_RET_OK;
}

/**
 * @fn int libgpio_write(uint8_t gpio, uint8_t value, uint8_t keepexported)
 * @brief Sets the GPIO value
 * @param gpio  The GPIO pin number whose value will be set
 * @param value The value of the pin to set (0 or 1)
 * @param keepexported  Whether or not to keep this pin exported to userspace (mandatory if pin is set to high)
 * @return LIBGPIO_RET_OK if no errors occurred; appropriate error code otherwise
 * 
 * This function sets the value of a given GPIO pin.
 */
int libgpio_write(uint8_t gpio, uint8_t value, uint8_t keepexported) {
    static char buf[30];
    const char *f = "01";
    ssize_t n;
    int fd, r;

    /* input sanity check */
    if (value > 1) return LIBGPIO_RET_INVALIDIN;

    /* build output file name */
    n = sprintf(buf, "/sys/class/gpio/gpio%u/value", gpio);

    /* check if the gpio is already exported */
    if (access(buf, F_OK))
        /* not already exported, let us try exporting it now... */
        if ((r = _libgpio_rwfile(gpio, _LIBGPIO_MODE_EXPORT)))
            /* failed to export */
            return r;

    /* set the gpio to write mode */
    if ((r = _libgpio_setdir(gpio, _LIBGPIO_MODE_OUT)))
        /* failed to enable writes */
        goto cleanup;

    /* open the file */
    fd = open(buf, O_WRONLY);
    /* No permission to write??? not sure why this would fail */
    if (fd == -1) { r = LIBGPIO_RET_SETGPIO_FAIL; goto cleanup; }

    /* write the value! */
    n = write(fd, f + value, 1);
    /* No permission to write??? not sure why this would fail */
    if (n == -1) { r = LIBGPIO_RET_SETGPIO_FAIL; goto cleanup; }

    /* done */
    close(fd);
    r = LIBGPIO_RET_OK;

cleanup:
    /* check if we want to keep the pin exported */
    if (!keepexported && !value)
        _libgpio_rwfile(gpio, _LIBGPIO_MODE_UNEXPORT); /* idc if this errors */

    return r;
}

/**
 * @fn int libgpio_read(uint8_t gpio, uint8_t *value, uint8_t keepexported)
 * @brief Reads the GPIO value
 * @param gpio  The GPIO pin number whose value will be set
 * @param value A pointer to the area to store the value of the pin
 * @param keepexported  Whether or not to keep this pin exported to userspace (will not change if the pin was already exported)
 * @return LIBGPIO_RET_OK if no errors occurred; appropriate error code otherwise
 * 
 * This function reads the value of a given GPIO pin.
 */
int libgpio_read(uint8_t gpio, uint8_t *value, uint8_t keepexported) {
    static char buf[30];
    ssize_t n;
    int fd, r;
    uint8_t exported = 0;

    /* build output file name */
    n = sprintf(buf, "/sys/class/gpio/gpio%u/value", gpio);

    /* check if the gpio is already exported */
    if (access(buf, F_OK)) {
        /* not already exported, let us try exporting it now... */
        if ((r = _libgpio_rwfile(gpio, _LIBGPIO_MODE_EXPORT)))
            /* failed to export */
            return r;
    } else { exported = 1; }

    /* check if we can read the gpio value */
    if (access(buf, R_OK))
        /* not able to read, let us try enabling reading now... */
        if ((r = _libgpio_setdir(gpio, _LIBGPIO_MODE_IN)))
            /* failed to enable reads */
            goto cleanup;

    /* open the file */
    fd = open(buf, O_RDONLY);
    /* No permission to read??? not sure why this would fail */
    if (fd == -1) { r = LIBGPIO_RET_READGPIO_FAIL; goto cleanup; }

    /* read the value! */
    n = read(fd, buf, 3);
    /* No permission to read??? not sure why this would fail */
    if (n == -1) { r = LIBGPIO_RET_READGPIO_FAIL; goto cleanup; }

    /* done */
    close(fd);

    /* parse out the value we read */
    *value = *buf - '0';
    r = LIBGPIO_RET_OK;

cleanup:
    /* check if we want to keep the pin exported */
    if (!keepexported && !exported)
        _libgpio_rwfile(gpio, _LIBGPIO_MODE_UNEXPORT); /* idc if this errors */

    return r;
}
