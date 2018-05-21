# rpigpio (Rest PI GPIO control)

GPIO control using a simple REST api!

Be easy to use and don't use up too many resources.
Pre-built binaries are available on the releases tab, so you don't have to install `build-essential` and run `make`. :)

I think this could be used on not just the raspberry pi, but my original use case was the Pi Zero W. You mileage with other boards may vary!

## API

All operations are on a specific GPIO `gpio`.
The `gpio` is the **GPIO number**, and **NOT the pin number**.
To see a mapping from GPIO number to pin number, please see [this website](https://www.raspberrypi-spy.co.uk/2012/06/simple-guide-to-the-rpi-gpio-header-and-pins/).

HTTPS is not supported, please use a proxy to secure requests and perform authorization!

### `GET /api/gpio/<gpio>`

Returns the current status of `gpio` as plain text (`0` for off, or `1` for on).

### `POST /api/gpio/<gpio>` OR `PUT /api/gpio/<gpio>`

Sets the status of `gpio` to a `status` (`0` for off, or `1` for on).

### `PATCH /api/gpio/<gpio>`

Toggles the current status of `gpio` for `time` seconds (if gpio was on before, it will be off momentarily and vice versa).
Responds after the entire toggle is complete; if you want a toggle greater than a few seconds, please use two requests of `PUT` instead.

## Running on boot

... TODO!

## Technical details

This program relies on 

## Credits and License

I heavily relied on [this document](https://www.kernel.org/doc/Documentation/gpio/sysfs.txt) whilst developing this program.

Thanks to the [yhs web server](https://github.com/tom-seddon/yhs) for providing the HTTP server part of this program.

For license information, please see the file `LICENSE` (TL;DR: public domain).

Thanks for checking out this project!
