# Station Mode Plus Soft Access Point

As a jumping off point for other projects, I needed the ability to turn on and off WIFI_STA and WIFI_AP independently.  This sketch demonstrates that functionality.

## Coverage

This has been tested with the ESP8266 and the ESP32.  No further work is expected.

## Usage

In `main.cpp` there are four timers which turn on and off Station and AP mode as an example:

``` cpp
    setAPConnect.once(10, []() { createAP(hostname); });
    setSTAConnect.once(20, []() { connectToAP(ssid, password, hostname); });
    setAPDestroy.once(30, []() { destroyAP(); });
    setSTADisconnect.once(40, []() { disconnectFromAP(); });
```

Included are web pages in `/data` which can be used as a demonstration.

The web server includes some rudimentray ability to determine if a given page is being served from the captive portal or normal service:

``` cpp
Log.verbose(F("Webserver: Serving %s, captive is %T." CR), _url, isCaptive);
```

## Secrets

The project expects certain local data to exist in a file named `secrets.h` in the `./src/` directory:

``` cpp
#pragma once
#ifndef _SECRET_H
#define _SECRET_H

#define _HOSTNAME "esp-captive"
#define _SSID "your_network"
#define _PASSWORD "your_password"
#define _HTTPPORT (uint16_t)80

#endif // _SECRET_H
```
