#ifndef Settings_h
#define Settings_h

#include <Arduino.h>

#include "GlobalStatus.h"

void settingsInit();

String settingsGetDeviceName();

String settingsGetWifiSsid();

String settingsGetWifiPassword();

Alarm settingsGetAlarm(uint number);

bool settingsGetInDeepSleep();

bool settingsSaveWifiSsid(String ssid);

bool settingsSaveWifiPassword(String password);

bool settingsSaveDeviceName(String name);

bool settingsSaveAlarm(Alarm alarm);

bool settingsSaveInDeepSleep(bool value);

#endif