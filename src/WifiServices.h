#ifndef WifiServices_h
#define WifiServices_h

#include <WString.h>

String getVerboseWifiStatus();
bool isWiFiConnected();
void disconnectWifi();
void connectWifi();
void initWifi();

#endif