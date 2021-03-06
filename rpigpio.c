#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "yhs.h"
#include "libgpio.h"

volatile sig_atomic_t done = 0;

void sighandle(int signum) {
    printf("Caught signal!\n");
    done = 1;
}

#define BUF_LEN 8
void handle_api(yhsRequest *re) {
    int x, delay_len;
    uint8_t gpio, val;
    const char *gpio_pin_str;
    yhsMethod method;
    char buf[BUF_LEN+1];

    /* get the gpio pin number */
    gpio_pin_str = yhs_get_path_handler_relative(re);
    x = atoi(gpio_pin_str);
    if (x < 0 || x > 256) return;
    gpio = (uint8_t)x;

    /* begin response */
    yhs_begin_data_response(re,"text/plain");

    /* get the method */
    method = yhs_get_method(re);

    switch (method) {
    case YHS_METHOD_GET:
        /* GET value of the GPIO pin */
        printf("[GET] gpio pin %u\n", gpio);
        x = libgpio_read(gpio, &val, 0);
        if (x) {
            printf("[GET] Error returned by libgpio_read(): code %d\n", x);
            yhs_text(re, "2");
        } else {
            printf("[GET] Success! Value is %d\n", val);
            yhs_textf(re, "%u", val);
        }
        break;
    case YHS_METHOD_PUT:
    case YHS_METHOD_POST:
        /* PUT value of the GPIO pin */
        /* read body */
        yhs_get_content_details(re,NULL,&x);
        if (x > BUF_LEN) x = BUF_LEN;
        yhs_get_content(re, x, buf);
        buf[x] = '\0';

        /* convert to integer */
        x = atoi(buf);
        if (x < 0 || x > 1) return; /* 0 or 1 only */
        val = (uint8_t)x;
        printf("[SET] gpio pin %u to %u\n", gpio, val);

        /* write value */
        x = libgpio_write(gpio, val, 0);
        if (x) {
            fprintf(stderr, "[SET] Error returned by libgpio_write(): code %d\n", x);
            yhs_text(re, "2");
        } else {
            printf("[SET] Success! Value is %d\n", val);
            yhs_textf(re, "%u", val);
        }
        break;
    case YHS_METHOD_PATCH:
        /* PATCH value of the GPIO pin */
        /* read body */
        yhs_get_content_details(re,NULL,&x);
        if (x > BUF_LEN) x = BUF_LEN;
        yhs_get_content(re, x, buf);
        buf[x] = '\0';

        /* convert to integer */
        delay_len = atoi(buf);
        if (delay_len < 0 || delay_len > 2000) return;
        printf("[TGL] gpio pin %u for %dms\n", gpio, delay_len);

        /* get value of the GPIO pin */
        x = libgpio_read(gpio, &val, 1);
        if (x) {
            fprintf(stderr, "[TGL] Error returned by libgpio_read(), can't read initial value of pin: code %d\n", x);
            yhs_text(re, "2");
            /* fatal error */
            break;
        }

        /* set new value */
        x = libgpio_write(gpio, !val, 1);
        if (x) {
            fprintf(stderr, "[TGL] Error returned by libgpio_write(), can't set pin %d to %u: code %d\n", gpio, !val, x);
            /* ignore-able error */
        }

        /* sleep */
        usleep(delay_len * 1000);

        /* return to old value */
        x = libgpio_write(gpio, val, 1);
        if (x) {
            fprintf(stderr, "[TGL] Error returned by libgpio_write(), can't set pin %d to %u: code %d\n", gpio, val, x);
            /* ignore-able error */
        }

        /* get value of the GPIO pin */
        x = libgpio_read(gpio, &val, 0);
        if (x) {
            fprintf(stderr, "[TGL] Error returned by libgpio_read(), can't read final value of pin: code %d\n", x);
            /* fatal error */
            yhs_text(re, "2");
            break;
        } else {
            printf("[TGL] Success! New value is %d\n", val);
            yhs_textf(re, "%u", val);
        }
        break;
    default:
        printf("wat\n");
        break;
    }
}

int main(int argc, char **argv) {
    int port;
    char *v;
    yhsServer *server;
    struct sigaction action;

    /* check if port given via args */
    if (argc == 2) {
        port = atoi(argv[1]);
        if (port > 0 && port < 65536) goto haveport;
    }
    /* invalid port given via args or no port given, now try env vars */
    v = getenv("RPIGPIO_PORT");
    if (v != NULL) {
        port = atoi(v);
        if (port > 0 && port < 65536) goto haveport;
    }
    port = 8080;
haveport:
    printf("Listening on port %d\n", port);

    /* catch SIGTERM */
    memset(&action, 0, sizeof(action));
    action.sa_handler = sighandle;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGTERM, &action, NULL) == -1) {
        printf("Failed to catch signals!");
    }

    /* create server */
    server = yhs_new_server(port);
    yhs_set_server_name(server, "rpigpio");
    //yhs_set_server_log_enabled(server, YHS_LOG_DEBUG, 1);

    /* add handlers */
    yhs_add_res_path_handler(server,"/api/gpio/",&handle_api,NULL);

    /* loop forever */
    while (!done) {
        yhs_update(server);
        usleep(1000);
    }

    printf("Quitting gracefully...\n");
    yhs_delete_server(server);
    return 0;
}
