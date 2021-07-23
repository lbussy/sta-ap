#include "mdnshandler.h"

static uint16_t httpPort = _HTTPPORT;
static bool mdnsRunning = false;

bool mdnsSetup()
{
    if (!MDNS.begin(WiFi.getHostname()))
    {
        Log.error(F("Error setting up mDNS responder." CR));
        return false;
    }
    else
    {
        if (!MDNS.addService(WiFi.getHostname(), "tcp", httpPort))
        {
            Log.error(F("Failed to register %s mDNS service." CR), WiFi.getHostname());
            return false;
        }
        else
        {
            Log.notice(F("Host %s registered via mDNS on port %i." CR), WiFi.getHostname(), httpPort);
        }
        if (!MDNS.addService("http", "tcp", httpPort))
        {
            Log.error(F("Failed to register Web mDNS service." CR));
            return false;
        }
        else
        {
            Log.notice(F("Web server registered via mDNS on port %i." CR), httpPort);
        }
        Log.notice(F("mDNS responder started for %s.local." CR), WiFi.getHostname());
        mdnsRunning = true;
        return true;
    }
}

void mdnsLoop()
{
#ifdef ESP8266
    MDNS.update();
#endif
}

bool mdnsIsRunning()
{
    return mdnsRunning;
}

bool mdnsStop()
{
    Log.notice(F("mDNS responder stopped for %s.local." CR), WiFi.getHostname());
    mdnsRunning = false;
#ifdef ESP8266
    return MDNS.end();
#elif defined ESP32
    MDNS.end();
    return true;
#endif
}
