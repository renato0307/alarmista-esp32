#ifndef GlobalStatus_h
#define GlobalStatus_h

#include <Arduino.h>

struct Alarm
{
    uint number;
    long when;
    String song;
    uint activeMatrix;
};

struct GlobalStatus
{
    bool isBleInitialized = false;
    bool goToSleep = false;
    bool goToConfig = false;
    bool goToSunrise = false;
    bool inDeepSleep = false;
    String lastOperationStatus = "";
    String wifiSsid = "";
    String wifiPassword = "";
    String deviceName = "";
};

extern GlobalStatus globalStatus;

#endif