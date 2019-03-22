#include "ConfigurationState.h"

#include <Arduino.h>
#include <ArduinoLog.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <StringSplitter.h>

#include "GlobalStatus.h"
#include "BLEServices.h"
#include "WifiServices.h"
#include "Settings.h"

#define CONFIGURATION_SERVICE_UUID                  "208cf64a-e85b-4f7e-9653-83aeb1c117c9"
#define WIFI_SET_SSID_CHARACTERISTIC_UUID           "cc0bd427-c9c3-43b0-a7c6-2df108b2b7c4"
#define WIFI_SET_PASSWORD_CHARACTERISTIC_UUID       "9200f537-e530-4f2a-b446-a5db5f3fc82f"
#define WIFI_INIT_CHARACTERISTIC_UUID               "b98d1c2c-cca5-44ee-b7d2-11e79a6b192e"
#define WIFI_STATUS_CHARACTERISTIC_UUID             "db041866-4f73-4e78-be6f-59ab4152b2a1"
#define ALARM_SET_CHARACTERISTIC_UUID               "34adb56d-e9fd-4892-816f-f3c31f1d0d98"
#define LAST_OPERATION_STATUS_CHARACTERISTIC_UUID   "d5821d4f-17b5-4c3a-b46c-d7fa23cb78f6"
#define GO_TO_SLEEP_CHARACTERISTIC_UUID             "9501faf3-b697-40de-ad74-0a10f5e2de2c"

const String LAST_OPERATION_STATUS_SUCCESS = "0";
const String LAST_OPERATION_STATUS_INVALID_ALARM_MISSING_FIELDS = "1";
const String LAST_OPERATION_STATUS_INVALID_ALARM_INVALID_FIELDS = "2";
const String LAST_OPERATION_STATUS_INVALID_ALARM_NOT_SAVED = "3";

/* ========================================================================= 
   Definitions
   ========================================================================= */

void saveAlarm(String value);

/* ========================================================================= 
   Private functions 
   ========================================================================= */

// LastOperationStatusBLEConfCallback handles last operation status fetch ble command
class LastOperationStatusBLEConfCallback : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    pCharacteristic->setValue(globalStatus.lastOperationStatus.c_str());
    Log.trace("returning last operation status: %s\n", globalStatus.lastOperationStatus.c_str());
  }
};

// AlarmSetBLEConfCallback handles alarm set ble command
class AlarmSetBLEConfCallback : public BLECharacteristicCallbacks
{

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    globalStatus.lastOperationStatus = LAST_OPERATION_STATUS_SUCCESS;
    String value = pCharacteristic->getValue().c_str();

    Log.trace("setting alarm: %s\n", value.c_str());
    saveAlarm(value);
  }
};

// WifiSetSsidBLEConfCallback handles wifi ssid set ble command
class WifiSetSsidBLEConfCallback : public BLECharacteristicCallbacks
{

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    globalStatus.lastOperationStatus = LAST_OPERATION_STATUS_SUCCESS;
    String value = pCharacteristic->getValue().c_str();

    Log.trace("setting wifi ssid to %s\n", value.c_str());
    globalStatus.wifiSsid = value;
  }
};

// WifiResetBLEConfCallback handles wifi reset ble command
class WifiResetBLEConfCallback : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    globalStatus.lastOperationStatus = LAST_OPERATION_STATUS_SUCCESS;
    String value = pCharacteristic->getValue().c_str();

    Log.trace("saving wifi credentials to persistent settings...\n");
    settingsSaveWifiSsid(globalStatus.wifiSsid);
    settingsSaveWifiPassword(globalStatus.wifiPassword);

    Log.trace("connecting to wifi...\n");
    disconnectWifi();
    initWifi();
  }
};

// WifiSetPasswordBLEConfCallback handles set wifi password ble command
class WifiSetPasswordBLEConfCallback : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    globalStatus.lastOperationStatus = LAST_OPERATION_STATUS_SUCCESS;
    String value = pCharacteristic->getValue().c_str();

    Log.trace("setting wifi password to %s\n", value.c_str());
    globalStatus.wifiPassword = value;
  }
};

// GoToSleepBLEConfCallback handles go to sleep ble command
class GoToSleepBLEConfCallback : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    Log.notice("entering sleep mode\n");
    
    xSemaphoreTake(globalStatus.sleepMutex, portMAX_DELAY);
    globalStatus.goToSleep = true;
    xSemaphoreGive(globalStatus.sleepMutex);
  }
};

// WifiStatusBLEConfCallback handles wifi status fetch ble command
class WifiStatusBLEConfCallback : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    String wifiStatus = getVerboseWifiStatus();
    pCharacteristic->setValue(wifiStatus.c_str());
    Log.trace("returning wifi status: %s\n", wifiStatus.c_str());
  }
};

// saveAlarm parses and saves an alarm received by BLE.
// The value must be in the following format:
// alarm number|date in unix epoch|song|active days
void saveAlarm(String value)
{
  StringSplitter *splitter = new StringSplitter(value, ',', 4);
  int itemCount = splitter->getItemCount();
  if (itemCount != 4)
  {
    Log.error("invalid value to set the alarm: %s\n", value.c_str());
    globalStatus.lastOperationStatus = LAST_OPERATION_STATUS_INVALID_ALARM_MISSING_FIELDS;
    return;
  }

  Alarm alarm;
  alarm.number = splitter->getItemAtIndex(0).toInt();
  alarm.when = splitter->getItemAtIndex(1).toInt();
  alarm.song = splitter->getItemAtIndex(2);
  alarm.activeMatrix = splitter->getItemAtIndex(3).toInt();
  if (alarm.number == 0 || alarm.when == 0 || alarm.activeMatrix == 0)
  {
    Log.error("number, when or activeMatrix must be integer bigger than 0: %s\n", value.c_str());
    globalStatus.lastOperationStatus = LAST_OPERATION_STATUS_INVALID_ALARM_INVALID_FIELDS;
    return;
  }

  if (!settingsSaveAlarm(alarm))
  {
    Log.error("could not save timer\n");
    globalStatus.lastOperationStatus = LAST_OPERATION_STATUS_INVALID_ALARM_NOT_SAVED;
  }
  Log.trace("alarm saved: %d %l %s %d\n", alarm.number, alarm.when, alarm.song.c_str(), alarm.activeMatrix);
}

// initBLE starts BLE services for the configuration state.
void initBLE()
{
  BLECharacteristicConf confs[] = {
      {WIFI_SET_SSID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE, new WifiSetSsidBLEConfCallback() },
      {WIFI_SET_PASSWORD_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE, new WifiSetPasswordBLEConfCallback() },
      {WIFI_INIT_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE, new WifiResetBLEConfCallback() },
      {WIFI_STATUS_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ, new WifiStatusBLEConfCallback() },
      {ALARM_SET_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE, new AlarmSetBLEConfCallback() },
      {LAST_OPERATION_STATUS_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ, new LastOperationStatusBLEConfCallback() },
      {GO_TO_SLEEP_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE, new GoToSleepBLEConfCallback() }};

  startBLE(CONFIGURATION_SERVICE_UUID, confs, sizeof(confs) / sizeof(BLECharacteristicConf));
}

// initDeviceName creates and saves the device name
// if no name is defined yet.
void initDeviceName()
{
  globalStatus.deviceName = settingsGetDeviceName();
  Log.trace("device name is [%s]\n", globalStatus.deviceName.c_str());

  if (globalStatus.deviceName != "")
    return;

  Log.trace("device name not set, generating a new one...\n");
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";
  int size = 3;                                
  char clientId[size + 1];
  for (uint8_t i = 0; i < size; i++)
  {
    clientId[i] = alphanum[random(62)];
  }
  clientId[size] = '\0';

  globalStatus.deviceName = "Alarmista " + String(clientId);
  settingsSaveDeviceName(globalStatus.deviceName);

  Log.trace("new device name is [%s]\n", globalStatus.deviceName.c_str());
}

/* ========================================================================= 
   Public functions 
   ========================================================================= */

// configurationStateLoop contains all the logic of the configuration state
void configurationStateLoop()
{
  Log.notice("=> entering state: Configuration\n");
  initDeviceName();
  initWifi();
  initBLE();
}

// configurationStateActivateSleep will return true when the configuration is done
// and the thing can exit configuration and enter sleep mode.
bool configurationStateActivateSleep()
{
  xSemaphoreTake(globalStatus.sleepMutex, portMAX_DELAY);

  Log.trace("going to sleep? %b\n", globalStatus.goToSleep);

  // if true goes to sleep but resets the value for the wake up
  bool goToSleepFlag = globalStatus.goToSleep;
  globalStatus.goToSleep = false; 

  xSemaphoreGive(globalStatus.sleepMutex);  

  return goToSleepFlag;
}