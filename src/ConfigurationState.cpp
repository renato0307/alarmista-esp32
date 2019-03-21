#include "ConfigurationState.h"

#include <Arduino.h>
#include <ArduinoLog.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <StringSplitter.h>

#include "GlobalStatus.h"

#define CONFIGURATION_SERVICE_UUID            "208cf64a-e85b-4f7e-9653-83aeb1c117c9"
#define WIFI_SET_SSID_CHARACTERISTIC_UUID     "cc0bd427-c9c3-43b0-a7c6-2df108b2b7c4"
#define WIFI_SET_PASSWORD_CHARACTERISTIC_UUID "9200f537-e530-4f2a-b446-a5db5f3fc82f"
#define WIFI_INIT_CHARACTERISTIC_UUID         "db041866-4f73-4e78-be6f-59ab4152b2a1"

/* ========================================================================= 
   Public functions 
   ========================================================================= */

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

// initBLE starts BLE services for the configuration state.
void initBLE()
{

  if (!globalStatus.isBleInitialized)
  {
    Log.trace("creating ble server\n");
    BLEDevice::init(globalStatus.deviceName.c_str());
    BLEServer *pServer = BLEDevice::createServer();

    Log.notice("creating ble service %s\n", CONFIGURATION_SERVICE_UUID);
    BLEService *pService = pServer->createService(CONFIGURATION_SERVICE_UUID);

    Log.notice("configuring characteristic %s\n", WIFI_SET_SSID_CHARACTERISTIC_UUID);
    BLECharacteristic *characteristic1 = pService->createCharacteristic(
        WIFI_SET_SSID_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    characteristic1->setCallbacks(new MyCallbacks());
    characteristic1->setValue("0");

    Log.notice("configuring characteristic %s\n", WIFI_SET_PASSWORD_CHARACTERISTIC_UUID);
    BLECharacteristic *characteristic2 = pService->createCharacteristic(
        WIFI_SET_PASSWORD_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    characteristic2->setCallbacks(new MyCallbacks());
    characteristic2->setValue("0");    

    Log.notice("configuring characteristic %s\n", WIFI_INIT_CHARACTERISTIC_UUID);
    BLECharacteristic *characteristic3 = pService->createCharacteristic(
        WIFI_INIT_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    characteristic3->setCallbacks(new MyCallbacks());
    characteristic3->setValue("0");  

    Log.trace("starting ble service and advertisements\n");
    pService->start();
    pServer->getAdvertising()->start();

    Log.trace("getting c1: %s\n", pService->getCharacteristic(WIFI_SET_SSID_CHARACTERISTIC_UUID)->getUUID().toString().c_str());
    Log.trace("getting c2: %s\n", pService->getCharacteristic(WIFI_SET_PASSWORD_CHARACTERISTIC_UUID)->getUUID().toString().c_str());
    Log.trace("getting c3: %s\n", pService->getCharacteristic(WIFI_INIT_CHARACTERISTIC_UUID)->getUUID().toString().c_str());

    globalStatus.isBleInitialized = true;

    Log.trace("listening on bluetooth as [%s]\n", globalStatus.deviceName.c_str());
  }
}


// configurationStateLoop contains all the logic of the configuration state
void configurationStateLoop()
{
  Log.trace("=> entering state: Configuration\n");
  initBLE();
}

// configurationStateActivateSleep will return true when the configuration is done
// and the thing can exit configuration and enter sleep mode.
bool configurationStateActivateSleep()
{
  return false;
}