
#include <WifiServices.h>

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoLog.h>

#include <GlobalStatus.h>
#include <Settings.h>

const String WIFI_STATUS_UNDEFINED = "not configured";
const String WIFI_STATUS_DISCONNECTED = "disconnected";
const String WIFI_STATUS_CONNECTED = "connected";

// isWiFiConnected returns true if the a WiFi connection is established;
// returns false otherwise
bool isWiFiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

// getVerboseWifiStatus returns a description of the current wifi status
String getVerboseWifiStatus()
{
  if (isWiFiConnected())
  {
    return WIFI_STATUS_CONNECTED;
  }

  if (globalStatus.wifiSsid != "" && globalStatus.wifiPassword != "")
  {
    return WIFI_STATUS_DISCONNECTED;
  }

  return WIFI_STATUS_UNDEFINED;
}

// disconnectWifi terminates an wifi connection if
// it's currently connected.
void disconnectWifi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Log.trace("wifi not connected\n");
    return;
  }

  bool result = WiFi.disconnect();
  Log.trace("wifi disconnected: %b\n", result);
}

// loadWifiCredentials reads wifi credentials from persistent storage
// and sets the global status.
void loadWifiCredentials()
{
  Log.trace("reading wifi credentials from persistent storage\n");
  globalStatus.wifiSsid = settingsGetWifiSsid();
  globalStatus.wifiPassword = settingsGetWifiPassword();
  Log.trace("wifi ssid is %s\n", globalStatus.wifiSsid.c_str());
  Log.trace("wifi password is %s\n", globalStatus.wifiPassword.c_str());
}

// connectWifi establishes the wifi connection with timeout support.
void connectWifi()
{
  Log.trace("connecting to %s\n", globalStatus.wifiSsid.c_str());
  WiFi.begin(globalStatus.wifiSsid.c_str(), globalStatus.wifiPassword.c_str());
  int maxTries = 0;
  while (!isWiFiConnected() && maxTries++ < 20)
  {
    delay(500);
    Log.trace("connecting to %s (check #%d)\n", globalStatus.wifiSsid.c_str(), maxTries);
  }
}

// initWifi established a wifi connection using the
// global status configurations.
void initWifi()
{
  if (isWiFiConnected())
  {
    Log.trace("connected to wifi, ip address is %s\n", WiFi.localIP().toString().c_str());
    return;
  }

  loadWifiCredentials();
  if (globalStatus.wifiSsid == "" || globalStatus.wifiPassword == "")
  {
    Log.trace("wifi not configured\n");
    return;
  }

  connectWifi();
  if (WiFi.status() != WL_CONNECTED)
  {
    Log.trace("wifi not connected\n");
    return;
  }

  Log.trace("wifi connected\n");
}
