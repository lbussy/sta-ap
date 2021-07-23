#include "main.h"

Ticker setAPConnect;
Ticker setSTAConnect;
Ticker setAPDestroy;
Ticker setSTADisconnect;

const char * hostname = _HOSTNAME;
const char * ssid = _SSID;
const char * password = _PASSWORD;

void setup()
{
    setSerial();
    wifiSetup();

    setAPConnect.once(10, []()
                      { createAP(hostname); });

    setSTAConnect.once(20, []()
                       { connectToAP(ssid, password, hostname); });

    setAPDestroy.once(30, []()
                      { destroyAP(); });

    setSTADisconnect.once(40, []()
                          { disconnectFromAP(); });
}

void loop()
{
    wifiLoop();
}
