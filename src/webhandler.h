#pragma once
#ifndef _CAPTIVE_H
#define _CAPTIVE_H

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <LittleFS.h>
#endif

#include "secret.h"
#include "setserial.h"

// Webserver Stuff
#include <ESPAsyncWebServer.h>
#include <ArduinoLog.h>
#include <Arduino.h>
void initWebServer();
bool webServerIsRunning();
void handlePages(AsyncWebServerRequest *request, bool isCaptive = false);
void stopWebServer();
const char *setContentType(const String &path);

#endif // _CAPTIVE_H
