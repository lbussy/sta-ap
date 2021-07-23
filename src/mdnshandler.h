#pragma once
#ifndef _MDNSHANDLER_H
#define _MDNSHANDLER_H

#ifdef ESP32
#include <ESPmDNS.h>
#include <WiFi.h>
#elif ESP8266
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif

#include "secret.h"
#include "setserial.h"

#include <ArduinoLog.h>
#include <Arduino.h>

bool mdnsSetup();
void mdnsLoop();
bool mdnsStop();
bool mdnsIsRunning();

#endif // _MDNSHANDLER_H
