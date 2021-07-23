#include "wifihandler.h"

DNSServer dnsServer;

static bool isAP = false;
static bool isSTA = false;

#ifdef ESP8266
// Subscribe to specific event and get event information as an argument to the callback
WiFiEventHandler stationModeConnected;
WiFiEventHandler stationModeDisconnected;
WiFiEventHandler stationModeAuthModeChanged;
WiFiEventHandler stationModeGotIP;
WiFiEventHandler stationModeDHCPTimeout;
WiFiEventHandler softAPModeStationConnected;
WiFiEventHandler softAPModeStationDisconnected;
WiFiEventHandler softAPModeProbeRequestReceived;
WiFiEventHandler wiFiModeChange;
#endif // ESP8266

void resetWiFi()
{ // Wipe wifi settings and reset controller
#ifdef ESP32
    WiFi.disconnect(true, true);
#elif ESP8266
    WiFi.disconnect(true);
#endif
    Log.notice(F("Restarting after clearing wifi settings." CR));
    ESP.restart();
}

void wifiSetup()
{
    setCallbacks();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    Log.notice(F("WiFi is off." CR));
}

bool createAP(const char *_ssid, const char *_password)
{
    if (WiFi.getMode() & WIFI_STA)
    {
        if (WiFi.mode(WIFI_AP_STA))
        {
            Log.notice(F("Set AP + STA mode." CR));
        }
        else
        {
            Log.error(F("Unable to set AP + STA mode." CR));
            return false;
        }
    }
    else
    {
        if (WiFi.mode(WIFI_AP))
        {
            Log.notice(F("Set AP mode." CR));
        }
        else
        {
            Log.error(F("Unable to set AP mode." CR));
            return false;
        }
    }
    if (WiFi.softAP(_ssid, _password, WiFi.channel()))
    {
        Log.notice(F("Started soft AP." CR));
        if (dnsServer.start(53, "*", WiFi.softAPIP()))
        {
            Log.notice(F("Started captive DNS." CR));
        }
        else
        {
            Log.error(F("Failed to start captive DNS." CR));
            return false;
        }
    }
    else
    {
        Log.error(F("Failed to start soft AP." CR));
        return false;
    }
    if (!webServerIsRunning())
        initWebServer();
    else
        Log.notice(F("Webserver available for captive portal." CR));
    isAP = true;
    return true;
}

bool connectToAP(const char *_ssid, const char *_password, const char*_hostname)
{
    if (WiFi.getMode() & WIFI_AP)
    {
        if (WiFi.mode(WIFI_AP_STA))
        {
            Log.notice(F("Set AP + STA mode." CR));
        }
        else
        {
            Log.error(F("Unable to set AP + STA mode." CR));
            return false;
        }
    }
    else
    {
        if (WiFi.mode(WIFI_STA))
        {
            Log.notice(F("Set STA mode." CR));
        }
        else
        {
            Log.error(F("Unable to set STA mode." CR));
            return false;
        }
    }
    WiFi.setHostname(_hostname);
    wl_status_t retval = WiFi.begin(_ssid, _password);
    Log.notice(F("Queued STA connection to %s, current status: %s (%d)." CR),
               _ssid,
               getWLStatusText((int)retval).c_str(),
               (int)retval);
    if (!webServerIsRunning())
        initWebServer();
    else
        Log.notice(F("Webserver available at http://%s.local." CR), _hostname);
    isSTA = true;
    return true;
}

bool destroyAP()
{
    Log.notice(F("Stopped captive DNS." CR));
    if (WiFi.getMode() & WIFI_STA)
    {
        if (WiFi.enableAP(false))
        {
            Log.notice(F("Disconnected soft AP." CR));
        }
        else
        {
            Log.error(F("Unable to disconnect soft AP." CR));
            return false;
        }
        if (WiFi.mode(WIFI_STA))
        {
            Log.notice(F("Downgraded to STA mode only." CR));
        }
        else
        {
            Log.error(F("Unable to downgrade to STA mode." CR));
            return false;
        }
    }
    else
    {
        if (webServerIsRunning())
            stopWebServer();
        if (WiFi.softAPdisconnect(true))
        {
            Log.notice(F("Disconnected soft AP." CR));
        }
        else
        {
            Log.error(F("Unable to disconnect soft AP." CR));
            return false;
        }
        if (WiFi.mode(WIFI_OFF))
        {
            Log.notice(F("Set WiFi mode to OFF" CR));
        }
        else
        {
            Log.error(F("Unable to set WiFi mode to OFF." CR));
        }
    }
    isAP = false;
    dnsServer.stop();
    return true;
}

bool disconnectFromAP()
{
    if (WiFi.getMode() & WIFI_AP)
    {
        if (WiFi.disconnect(false))
        {
            Log.notice(F("Disconnected from AP." CR));
            if (WiFi.mode(WIFI_AP))
            {
                Log.notice(F("Set WiFi mode to Soft AP" CR));
            }
            else
            {
                Log.error(F("Unable to set WiFi mode to Soft AP." CR));
                return false;
            }
        }
        else
        {
            Log.error(F("Unable to disconnect from AP." CR));
            return false;
        }
    }
    else
    {
        if (webServerIsRunning())
            stopWebServer();
        if (WiFi.mode(WIFI_OFF))
        {
            Log.notice(F("Set WiFi mode to OFF" CR));
        }
        else
        {
            Log.error(F("Unable to set WiFi mode to OFF." CR));
        }
    }
    isSTA = false;
    mdnsStop();
    return true;
}

void wifiLoop()
{
    if (isAP)
        dnsServer.processNextRequest();
    if (isSTA)
        mdnsLoop();
}

void setCallbacks()
{
#ifdef ESP8266 // ESP8266
    setESP8266Callbacks();
#else
    setESP32Callbacks();
#endif // ESP8266
}

#ifdef ESP8266
void setESP8266Callbacks()
{
    // Subscribe to specific event and get event information as an argument to the callback
    stationModeConnected = WiFi.onStationModeConnected(&onStationModeConnected);
    stationModeDisconnected = WiFi.onStationModeDisconnected(&onStationModeDisconnected);
    stationModeAuthModeChanged = WiFi.onStationModeAuthModeChanged(&onStationModeAuthModeChanged);
    stationModeGotIP = WiFi.onStationModeGotIP(&onStationModeGotIP);
    stationModeDHCPTimeout = WiFi.onStationModeDHCPTimeout(&onStationModeDHCPTimeout);
    softAPModeStationConnected = WiFi.onSoftAPModeStationConnected(&onSoftAPModeStationConnected);
    softAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected(&onSoftAPModeStationDisconnected);
    // softAPModeProbeRequestReceived = WiFi.onSoftAPModeProbeRequestReceived(&onSoftAPModeProbeRequestReceived);
    wiFiModeChange = WiFi.onWiFiModeChange(&onWiFiModeChange);
}

void onStationModeConnected(const WiFiEventStationModeConnected &evt)
{
    // String ssid;
    // uint8 bssid[6];
    // uint8 channel;
    Log.notice(F("[WiFiEventStationModeConnected] Station mode connected to %s on channel %d, BSSID: %s." CR),
               evt.ssid.c_str(),
               evt.channel,
               macToString(evt.bssid).c_str());
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected &evt)
{
    // String ssid;
    // uint8 bssid[6];
    // WiFiDisconnectReason reason;
    Log.notice(F("[WiFiEventStationModeDisconnected] Disconnected from %s(%s): %s." CR),
               evt.ssid.c_str(),
               macToString(evt.bssid).c_str(),
               getWiFiDisconnectReasonText((int)evt.reason).c_str());
}

void onStationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged &evt)
{
    // uint8 oldMode;
    // uint8 newMode;
    // String mode[4] = { "WIFI_OFF", "WIFI_STA", "WIFI_AP", "WIFI_AP_STA" };
    Log.notice(F("[WiFiEventStationModeAuthModeChanged] WiFi mode changed from %s (%d) to %s (%d)." CR),
               getWiFiAuthModeText((int)evt.oldMode).c_str(),
               evt.oldMode,
               getWiFiAuthModeText((int)evt.newMode).c_str(),
               evt.newMode);
}

void onStationModeGotIP(const WiFiEventStationModeGotIP &evt)
{
    // IPAddress ip;
    // IPAddress mask;
    // IPAddress gw;
    Log.notice(F("[WiFiEventStationModeGotIP] Received IP: %p, SN: %p, GW: %p." CR), evt.ip, evt.mask, evt.gw);
    mdnsSetup();
}

void onStationModeDHCPTimeout()
{
    // No arguments passed
    Log.notice(F("[WiFiEventStationModeDHCPTimeout] DHCP request timed out." CR));
}

void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected &evt)
{
    // uint8 mac[6];
    // uint8 aid;
    Log.notice(F("[WiFiEventSoftAPModeStationConnected] Station (%d) connected: %s" CR), evt.aid, macToString(evt.mac).c_str());
}

void onSoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected &evt)
{
    // uint8 mac[6];
    // uint8 aid;
    Log.notice(F("[WiFiEventSoftAPModeStationDisconnected] Station (%d) disconnected: %s" CR), evt.aid, macToString(evt.mac).c_str());
}

void onSoftAPModeProbeRequestReceived(const WiFiEventSoftAPModeProbeRequestReceived &evt)
{
    // int rssi;
    // uint8 mac[6];
    Log.notice(F("[WiFiEventSoftAPModeProbeRequestReceived] Probe request from %s, RSSI: %d" CR),
               macToString(evt.mac).c_str(),
               evt.rssi);
}

void onWiFiModeChange(const WiFiEventModeChange &evt)
{
    // WiFiMode oldMode;
    // WiFiMode newMode;
    // TODO:  Either this or the getWiFiModeText() is wrong
    Log.notice(F("[WiFiEventModeChange] WiFi mode changed to %s (%d)." CR),
               getWiFiModeText(WiFi.getMode()).c_str(),
               WiFi.getMode());
}

String getWiFiModeText(int mode)
{
    switch (mode)
    {
    case 0:
        return "WIFI_OFF";
    case 1:
        return "WIFI_STA";
    case 2:
        return "WIFI_AP";
    case 3:
        return "WIFI_AP_STA";
    default:
        return "WIFI_UNKNOWN";
    }
}
#elif defined ESP32
void setESP32Callbacks()
{
    // Subscribe to specific event
    WiFi.onEvent(onWifiReady, SYSTEM_EVENT_WIFI_READY);
    WiFi.onEvent(onScanDone, SYSTEM_EVENT_SCAN_DONE);
    WiFi.onEvent(onStaStart, SYSTEM_EVENT_STA_START);
    WiFi.onEvent(onStaStop, SYSTEM_EVENT_STA_STOP);
    WiFi.onEvent(onStaConnected, SYSTEM_EVENT_STA_CONNECTED);
    WiFi.onEvent(onStaDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
    WiFi.onEvent(onStaAuthmodeChange, SYSTEM_EVENT_STA_AUTHMODE_CHANGE);
    WiFi.onEvent(onStaGotIP, SYSTEM_EVENT_STA_GOT_IP);
    WiFi.onEvent(onStaLost_IP, SYSTEM_EVENT_STA_LOST_IP);
    WiFi.onEvent(onStaWPSErSuccess, SYSTEM_EVENT_STA_WPS_ER_SUCCESS);
    WiFi.onEvent(onStaWPSErFailed, SYSTEM_EVENT_STA_WPS_ER_FAILED);
    WiFi.onEvent(onStaWPSErTimeout, SYSTEM_EVENT_STA_WPS_ER_TIMEOUT);
    WiFi.onEvent(onStaWPSErPin, SYSTEM_EVENT_STA_WPS_ER_PIN);
    WiFi.onEvent(onStaWPSErPbcOverlap, SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP);
    WiFi.onEvent(onAPStart, SYSTEM_EVENT_AP_START);
    WiFi.onEvent(onAPStop, SYSTEM_EVENT_AP_STOP);
    WiFi.onEvent(onAPStaConnected, SYSTEM_EVENT_AP_STACONNECTED);
    WiFi.onEvent(onAPStaDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);
    WiFi.onEvent(onAPStaIPAssigned, SYSTEM_EVENT_AP_STAIPASSIGNED);
    WiFi.onEvent(onAPProbeReqRecved, SYSTEM_EVENT_AP_PROBEREQRECVED);
    WiFi.onEvent(onGotIP6, SYSTEM_EVENT_GOT_IP6);
}

void onWifiReady(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 WiFi ready */
    Log.notice(F("[SYSTEM_EVENT_WIFI_READY] WiFi ready." CR));
}

void onScanDone(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 finish scanning AP */
    Log.notice(F("[SYSTEM_EVENT_SCAN_DONE] AP scan complete." CR));
}

void onStaStart(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station start */
    Log.notice(F("[SYSTEM_EVENT_STA_START] Station mode started." CR));
}

void onStaStop(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station stop */
    Log.notice(F("[SYSTEM_EVENT_STA_STOP] Station mode stopped." CR));
}

void onStaConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station connected to AP */
    Log.notice(F("[SYSTEM_EVENT_STA_CONNECTED] Station mode connected to %s on channel %d, BSSID: %s." CR),
               info.connected.ssid,
               info.connected.channel,
               macToString(info.connected.bssid).c_str());
}

void onStaDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station disconnected from AP */
    Log.notice(F("[SYSTEM_EVENT_STA_DISCONNECTED] Station disconnected from %s (%s): %s." CR),
               info.disconnected.ssid,
               info.disconnected.bssid,
               getWiFiDisconnectReasonText(info.disconnected.reason).c_str());
}

void onStaAuthmodeChange(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< the auth mode of AP connected by ESP32 station changed */
    Log.notice(F("[SYSTEM_EVENT_STA_AUTHMODE_CHANGE] Authmode changed from %s to %s." CR),
               getWiFiAuthModeText(info.auth_change.new_mode).c_str(),
               getWiFiAuthModeText(info.auth_change.old_mode).c_str());
}

void onStaGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station got IP from connected AP */
    Log.notice(F("[SYSTEM_EVENT_STA_GOT_IP] Received IP: %p, SN: %p, GW: %p." CR),
               IPAddress(info.got_ip.ip_info.ip.addr),
               IPAddress(info.got_ip.ip_info.gw.addr),
               IPAddress(info.got_ip.ip_info.netmask.addr));
    mdnsSetup();
}

void onStaLost_IP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station lost IP and the IP is reset to 0 */
    Log.notice(F("[SYSTEM_EVENT_STA_LOST_IP] Station lost IP address." CR));
}

void onStaWPSErSuccess(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station wps succeeds in enrollee mode */
    Log.notice(F("[SYSTEM_EVENT_STA_WPS_ER_SUCCESS]" CR));
}

void onStaWPSErFailed(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station wps fails in enrollee mode */
    Log.notice(F("[SYSTEM_EVENT_STA_WPS_ER_FAILED]" CR));
}

void onStaWPSErTimeout(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station wps timeout in enrollee mode */
    Log.notice(F("[SYSTEM_EVENT_STA_WPS_ER_TIMEOUT]" CR));
}

void onStaWPSErPin(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station wps pin code in enrollee mode */
    Log.notice(F("[SYSTEM_EVENT_STA_WPS_ER_PIN]" CR));
}

void onStaWPSErPbcOverlap(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /*!< ESP32 station wps overlap in enrollee mode */
    Log.notice(F("[SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP]" CR));
}

void onAPStart(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 soft-AP start */
    Log.notice(F("[SYSTEM_EVENT_AP_START] Soft AP started." CR));
    // TODO:  Put more information here?
}

void onAPStop(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 soft-AP stop */
    Log.notice(F("[SYSTEM_EVENT_AP_STOP] Soft AP stopped." CR));
}

void onAPStaConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< a station connected to ESP32 soft-AP */
    Log.notice(F("[SYSTEM_EVENT_AP_STACONNECTED] Station %s (%d) connected to soft AP" CR),
               macToString(info.sta_connected.mac).c_str(),
               info.sta_connected.aid);
}

void onAPStaDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< a station disconnected from ESP32 soft-AP */
    Log.notice(F("[SYSTEM_EVENT_AP_STADISCONNECTED] Station %s (%d) disconnected from soft AP." CR),
               macToString(info.sta_disconnected.mac).c_str(),
               info.sta_disconnected.aid);
}

void onAPStaIPAssigned(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 soft-AP assign an IP to a connected station */
    Log.notice(F("[SYSTEM_EVENT_AP_STAIPASSIGNED] Station connected to soft AP, assigned %p." CR),
               IPAddress(info.ap_staipassigned.ip.addr));
}

void onAPProbeReqRecved(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< Receive probe request packet in soft-AP interface */
    Log.notice(F("[SYSTEM_EVENT_AP_PROBEREQRECVED]" CR));
}

void onGotIP6(WiFiEvent_t event, WiFiEventInfo_t info)
{
    /**< ESP32 station or ap or ethernet interface v6IP addr is preferred */
    Log.notice(F("[SYSTEM_EVENT_GOT_IP6]" CR));
}
#endif // ESP8266 / ESP32

String getWLStatusText(int status)
{
    switch (status)
    {
    case 0:
        return "WL_IDLE_STATUS";
    case 1:
        return "WL_NO_SSID_AVAIL";
    case 2:
        return "WL_SCAN_COMPLETED";
    case 3:
        return "WL_CONNECTED";
    case 4:
        return "WL_CONNECT_FAILED";
    case 5:
        return "WL_CONNECTION_LOST";
    case 6:
        return "WL_DISCONNECTED";
    default:
        return "WL_UNKNOWN";
    }
}

String getWiFiDisconnectReasonText(int reason)
{
    // From: framework-arduinoespressif8266\libraries\ESP8266WiFi\src\ESP8266WiFiType.h
    switch (reason)
    {
    case 1:
        return "WIFI_DISCONNECT_REASON_UNSPECIFIED";
    case 2:
        return "WIFI_DISCONNECT_REASON_AUTH_EXPIRE";
    case 3:
        return "WIFI_DISCONNECT_REASON_AUTH_LEAVE";
    case 4:
        return "WIFI_DISCONNECT_REASON_ASSOC_EXPIRE";
    case 5:
        return "WIFI_DISCONNECT_REASON_ASSOC_TOOMANY";
    case 6:
        return "WIFI_DISCONNECT_REASON_NOT_AUTHED";
    case 7:
        return "WIFI_DISCONNECT_REASON_NOT_ASSOCED";
    case 8:
        return "WIFI_DISCONNECT_REASON_ASSOC_LEAVE";
    case 9:
        return "WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED";
    case 10: /* 11h */
        return "WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD";
    case 11: /* 11h */
        return "WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD";
    case 13: /* 11h */
        return "WIFI_DISCONNECT_REASON_IE_INVALID";
    case 14: /* 11h */
        return "WIFI_DISCONNECT_REASON_MIC_FAILURE";
    case 15: /* 11h */
        return "WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT";
    case 16: /* 11h */
        return "WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT";
    case 17: /* 11h */
        return "WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS";
    case 18: /* 11h */
        return "WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID";
    case 19: /* 11h */
        return "WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID";
    case 20: /* 11h */
        return "WIFI_DISCONNECT_REASON_AKMP_INVALID";
    case 21: /* 11h */
        return "WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION";
    case 22: /* 11h */
        return "WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP";
    case 23: /* 11h */
        return "WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED";
    case 24: /* 11h */
        return "WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED";
    case 200:
        return "WIFI_DISCONNECT_REASON_BEACON_TIMEOUT";
    case 201:
        return "WIFI_DISCONNECT_REASON_NO_AP_FOUND";
    case 202:
        return "WIFI_DISCONNECT_REASON_AUTH_FAIL";
    case 203:
        return "WIFI_DISCONNECT_REASON_ASSOC_FAIL";
    case 204:
        return "WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT";
    default:
        return "WIFI_DISCONNECT_UNKNOWN";
    }
}

String getWiFiAuthModeText(int mode)
{
    switch (mode)
    {
    case 0:
        return "WIFI_AUTH_OPEN";
    case 1:
        return "WIFI_AUTH_WEP";
    case 2:
        return "WIFI_AUTH_WPA_PSK";
    case 3:
        return "WIFI_AUTH_WPA2_PSK";
    case 4:
        return "WIFI_AUTH_WPA_WPA2_PSK";
    case 5:
        return "WIFI_AUTH_WPA2_ENTERPRISE";
    default:
        return "WIFI_AUTH_UNKNOWN";
    }
}

String macToString(const unsigned char *mac)
{
    char buf[20];
    snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}
