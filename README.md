# Humidity & temperature app for Mono

This app works with the DHT22 (AM2302) and DHT11 humidity and temperature sensors.

See [the tutorial](http://developer.openmono.com/en/latest/tutorials/tutorials.html#humidty-app) on the [developer site](http://developer.openmono.com) for a an explanation of how (a simplified version of) this app works.

## Uploading data

You can make the app track the humidity & temperature for longer periods of time, and upload the readings to [our graph service](http://grapher.openmono.com).

To do that, you obviously need a Mono with Wifi, and you need to configure your Mono.

## Configuration

If you want to upload readings to the [graph service](http://grapher.openmono.com), you need tell Mono where exactly to upload your data so that they do not collide with other peoples data in the graph service.

You will need the following directories and file on an SD card:

    /mono/wifi/ssid.txt
    /mono/wifi/password.txt
    /mono/humidity/dataset/path.txt

All the above files should be a single line of text without newlines.  The `path.txt` should be some unix-style path that is uniquely to you, like

    /acme-corp/jane/temp

which would place your readings on

    https://grapher.openmono.com/data/acme-corp/jane/temp

## Download the compiled app

Follow the instructions on [the app page](http://kiosk.openmono.com/app/com-openmono-humidity) on MonoKiosk.
