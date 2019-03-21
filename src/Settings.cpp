#include <Preferences.h>
#include "Settings.h"

const int MAX_ALARMS = 4;

const String DEVICE_NAME = "device-name";
const String WIFI_SSI = "wifi-ssid";
const String WIFI_PASSWORD = "wifi-password";

const String ALARM_NUMBER = "alarm-number-";
const String ALARM_WHEN = "alarm-when-";
const String ALARM_SONG = "alarm-song-";
const String ALARM_ACTIVE = "alarm-active-";

Preferences preferences;

// settingsInit needs to be called (maybe in setup)
// to allow settings to be used
void settingsInit()
{
    preferences.begin("settings");
}

// settingsGetDeviceName returns the device name stored in the preferences
String settingsGetDeviceName()
{
    return preferences.getString(DEVICE_NAME.c_str());
}

// settingsGetWifiSsid returns the wifi ssid stored in the preferences
String settingsGetWifiSsid()
{
    return preferences.getString(WIFI_SSI.c_str());
}

// settingsGetWifiPassword returns the wifi password stored in the preferences
String settingsGetWifiPassword()
{
    return preferences.getString(WIFI_PASSWORD.c_str());
}

// settingsGetAlarm returns an alarm stored in the preferences
Alarm settingsGetAlarm(uint number)
{
    Alarm alarm;
    if (number > MAX_ALARMS || number <= 0)
    {
        return alarm;
    }

    alarm.number = preferences.getUInt((ALARM_NUMBER + String(number)).c_str());
    alarm.when = preferences.getULong((ALARM_WHEN + String(number)).c_str());
    alarm.song = preferences.getString((ALARM_WHEN + String(number)).c_str());
    alarm.activeMatrix = preferences.getUInt((ALARM_ACTIVE + String(number)).c_str());

    return alarm;
}

// settingsSaveDeviceName stores the device name in the preferences
bool settingsSaveDeviceName(String name)
{
    return preferences.putString(DEVICE_NAME.c_str(), name) > 0;
}

// settingsSaveWifiSsid stores the wifi ssid in the preferences
bool settingsSaveWifiSsid(String ssid)
{
    return preferences.putString(WIFI_SSI.c_str(), ssid) > 0;
}

// settingsSaveWifiPassword stores the wifi password in the preferences
bool settingsSaveWifiPassword(String password)
{
    return preferences.putString(WIFI_PASSWORD.c_str(), password) > 0;
}

// settingsSaveAlarm stores an alarm in the preferences
bool settingsSaveAlarm(Alarm alarm)
{
    uint number = alarm.number;
    if (number > MAX_ALARMS || number <= 0)
    {
        return false;
    }

    return preferences.putUInt((ALARM_NUMBER + String(number)).c_str(), alarm.number) > 0 
        && preferences.putULong((ALARM_WHEN + String(number)).c_str(), alarm.when) > 0 
        && preferences.putString((ALARM_WHEN + String(number)).c_str(), alarm.song) > 0 
        && preferences.putUInt((ALARM_ACTIVE + String(number)).c_str(), alarm.activeMatrix) > 0;
}