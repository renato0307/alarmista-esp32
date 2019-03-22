#ifndef GlobalStatus_h
#define GlobalStatus_h

#include <Arduino.h>

struct Alarm {
    uint number;
    long when;
    String song;
    uint activeMatrix;
};

struct GlobalStatus {
    bool isBleInitialized = false;
    bool goToSleep = false;
    String lastOperationStatus = "";
    String wifiSsid = "";
    String wifiPassword = "";
    String deviceName = "";
    SemaphoreHandle_t sleepMutex;
};

extern GlobalStatus globalStatus;

#endif