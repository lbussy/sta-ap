#pragma once
#ifndef _WIFIHANDLER_H
#define _WIFIHANDLER_H

#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif

#include "secret.h"
#include "mdnshandler.h"
#include "webhandler.h"
#include "setserial.h"
#include <ArduinoLog.h>
#include <DNSServer.h>
#include <Ticker.h> // DEBUG

void resetWiFi();
bool connectToAP(const char *_ssid, const char *_password, const char*_hostname);
bool createAP(const char * ssid, const char * password = NULL);
bool destroyAP();
bool disconnectFromAP();
void wifiSetup();
void wifiLoop();
void setCallbacks();

#ifdef ESP8266
// ESP8266 Callbacks:
void setESP8266Callbacks();
void onStationModeConnected(const WiFiEventStationModeConnected& evt);
void onStationModeDisconnected(const WiFiEventStationModeDisconnected& evt);
void onStationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged& evt);
void onStationModeGotIP(const WiFiEventStationModeGotIP& evt);
void onStationModeDHCPTimeout();
void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected& evt);
void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt);
void onSoftAPModeProbeRequestReceived(const WiFiEventSoftAPModeProbeRequestReceived& evt);
void onWiFiModeChange(const WiFiEventModeChange& evt);
String getWiFiModeText(int mode);
#elif defined ESP32
// ESP32 Callbacks:
void setESP32Callbacks();
void onWifiReady(WiFiEvent_t event, WiFiEventInfo_t info);
void onScanDone(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaStart(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaStop(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaAuthmodeChange(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaLost_IP(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaWPSErSuccess(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaWPSErFailed(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaWPSErTimeout(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaWPSErPin(WiFiEvent_t event, WiFiEventInfo_t info);
void onStaWPSErPbcOverlap(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStart(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStop(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStaConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStaDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStaIPAssigned(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPProbeReqRecved(WiFiEvent_t event, WiFiEventInfo_t info);
void onGotIP6(WiFiEvent_t event, WiFiEventInfo_t info);
#endif // ESP8266 // ESP32

String getWiFiDisconnectReasonText(int reason);
String getWLStatusText(int status);
String getWiFiAuthModeText(int mode);
String macToString(const unsigned char* mac);

struct tcp_pcb;
extern struct tcp_pcb *tcp_tw_pcbs;
extern "C" void tcp_abort(struct tcp_pcb *pcb);
void tcpCleanup(void);

#endif // _WIFIHANDLER_H
