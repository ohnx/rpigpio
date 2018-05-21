# rpigpio (Rest PI GPIO control)

GPIO control using a simple REST api!

The goal of this project is to be easy to use and don't use up too many resources.
Pre-built binaries are available on the releases tab, so you don't have to install `build-essential` and run `make`. :)

I think this could be used on not just the raspberry pi, but my original use case was the Pi Zero W.
Your mileage with other boards may vary!

## API

All operations are on a specific GPIO `gpio`.
The `gpio` is the **GPIO number**, and **NOT the pin number**.
To see a mapping from GPIO number to pin number, please see [this website](https://www.raspberrypi-spy.co.uk/2012/06/simple-guide-to-the-rpi-gpio-header-and-pins/).

HTTPS is not supported, please use a proxy to secure requests and perform authorization!

### `GET /api/gpio/<gpio>`

Returns the current status of `gpio` as plain text (`0` for off, or `1` for on).

### `POST /api/gpio/<gpio>` OR `PUT /api/gpio/<gpio>`

Sets the status of `gpio` to request body (`0` for off, or `1` for on).
Returns the new status of the gpio pin.

e.g. request:

```
POST /api/gpio/2 HTTP/1.0
User-Agent: Something/1.0
Content-Type: text/plain
Content-Length: 1

1
```


### `PATCH /api/gpio/<gpio>`

Toggles the current status of `gpio` for request body miliseconds (if gpio was on before, it will be off momentarily and vice versa).
Responds the new status of the gpio pin after the entire toggle is complete; if you want a toggle greater than 2 seconds, please use two requests of `PUT` instead.
(In fact, the request will return an error if the value is greater than 2000 or less than 0)

```
PATCH /api/gpio/2 HTTP/1.0
User-Agent: Something/1.0
Content-Type: text/plain
Content-Length: 4

1000
```

On error, 2 is returned.

### Invalid input

Only GPIO pins under 256 are supported. Invalid values will return a 404 error.

### Errors

Errors are logged to stderr on the server and will result in a value of 2 being returned.

## Running on boot

... TODO!

## Technical details

This program relies on the sysfs interface to do stuff with GPIO pins.

## TODO

* Test the yhs server for common exploits, e.g. Content-Length is like 798457348957 but only send 10 bytes and vice versa
* Check for memory leaks with yhs

## Credits and License

I heavily relied on [this document](https://www.kernel.org/doc/Documentation/gpio/sysfs.txt) whilst developing this program.

Thanks to the [yhs web server](https://github.com/tom-seddon/yhs) for providing the HTTP server part of this program.

For license information, please see the file `LICENSE` (TL;DR: public domain).

Thanks for checking out this project!
