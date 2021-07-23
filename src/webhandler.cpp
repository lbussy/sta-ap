#include "webhandler.h"

static uint16_t httpPort = _HTTPPORT;


#ifdef ESP32
#define FILESYSTEM SPIFFS
#elif ESP8266
#define FILESYSTEM LittleFS
#endif

AsyncWebServer webServer(httpPort);

static bool webRunning = false;

// Webserver Stuff:

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        handlePages(request, true);
    }
};

class StationRequestHandler : public AsyncWebHandler
{
public:
    StationRequestHandler() {}
    virtual ~StationRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        handlePages(request, false);
    }
};

constexpr unsigned int str2int(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

void initWebServer()
{
    FILESYSTEM.begin();
    webServer.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);  // Only when requested from AP
    webServer.addHandler(new StationRequestHandler()).setFilter(ON_STA_FILTER); // Only when requested from STA
    webServer.begin();
    webRunning = true;
    if (WiFi.getMode() & WIFI_STA)
        Log.notice(F("Webserver started at http://%s.local." CR), WiFi.getHostname());
    else if (WiFi.getMode() & WIFI_AP)
        Log.notice(F("Webserver started for captive portal." CR));
    else
        Log.warning(F("Web server started with an unknown access method." CR));
}

bool webServerIsRunning()
{
    return webRunning;
}

void handlePages(AsyncWebServerRequest *request, bool isCaptive)
{
    AsyncWebServerResponse *response;
    const char *_url = request->url().c_str();
    String file;

    Log.verbose(F("Webserver: Serving %s, captive is %T." CR), _url, isCaptive);

    if (FILESYSTEM.exists(_url) && strcmp(_url, "/") != 0)
    {
        Log.verbose(F("Serving %s from filesystem." CR), _url);
        file = _url;
    }
    else
    {
        Log.verbose(F("Serving %s from switch handlers." CR), _url);
        switch (str2int(_url))
        {
        case str2int("/"):
            file = "/index.html";
            break;
        case str2int("/index/"):
            file = "/index.html";
            break;
        case str2int("/doportal/"):
            // TODO:  Do something here
            file = "/doportal.html";
            break;
        case str2int("/another/"):
            file = "/another.html";
            break;
        default:
            if (isCaptive)
                file = "/index.html";
            else
                file = "/404.html";
            break;
        }
    }

    response = request->beginResponse(FILESYSTEM, file, setContentType(_url));
    response->addHeader("Cache-Control", "no-store"); // DEBUG
    if (isCaptive)
        response->addHeader("Redirect-Header", "true");
    else
        response->addHeader("Redirect-Header", "false");
    request->send(response);
}

void stopWebServer()
{
    webRunning = false;
    webServer.end();
    Log.notice(F("Webserver stopped." CR));
}

const char *setContentType(const String &path)
{
    if (path.endsWith("/"))
        return "text/html";
    else if (path.endsWith(".html"))
        return "text/html";
    else if (path.endsWith(".htm"))
        return "text/html";
    else if (path.endsWith(".css"))
        return "text/css";
    else if (path.endsWith(".json"))
        return "application/json";
    else if (path.endsWith(".js"))
        return "application/javascript";
    else if (path.endsWith(".png"))
        return "image/png";
    else if (path.endsWith(".gif"))
        return "image/gif";
    else if (path.endsWith(".jpg"))
        return "image/jpeg";
    else if (path.endsWith(".ico"))
        return "image/x-icon";
    else if (path.endsWith(".svg"))
        return "image/svg+xml";
    else if (path.endsWith(".eot"))
        return "font/eot";
    else if (path.endsWith(".woff"))
        return "font/woff";
    else if (path.endsWith(".woff2"))
        return "font/woff2";
    else if (path.endsWith(".ttf"))
        return "font/ttf";
    else if (path.endsWith(".xml"))
        return "text/xml";
    else if (path.endsWith(".pdf"))
        return "application/pdf";
    else if (path.endsWith(".zip"))
        return "application/zip";
    else if (path.endsWith(".gz"))
        return "application/x-gzip";
    else
        return "text/plain";
}

// Webserver Stuff^
