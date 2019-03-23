#include <BLEServices.h>

#include <ArduinoLog.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <GlobalStatus.h>

// initCharacteristic initializes a characteristic and assigns it to a service.
void initCharacteristic(BLEService *pService, String uuid, uint32_t properties, BLECharacteristicCallbacks *callbacks)
{
  BLECharacteristic *characteristic = pService->createCharacteristic(uuid.c_str(), properties);
  characteristic->setCallbacks(callbacks);
  characteristic->setValue("");
}

// startBLE is used to start the BLE server and expose services and characteristics
void startBLE(String serviceUuid, BLECharacteristicConf confs[], int confsSize)
{
  if (!globalStatus.isBleInitialized)
  {

    Log.trace("creating ble server\n");
    BLEDevice::init(globalStatus.deviceName.c_str());
    BLEServer *pServer = BLEDevice::createServer();

    Log.trace("creating ble service %s\n", serviceUuid.c_str());
    BLEService *pService = pServer->createService(serviceUuid.c_str());

    Log.trace("configuring %d ble characteristics\n", confsSize);
    for (int i = 0; i < confsSize; i++)
    {
      Log.trace("configuring characteristic %s\n", confs[i].uuid.c_str());
      initCharacteristic(
          pService,
          confs[i].uuid,
          confs[i].properties,
          confs[i].callbacks);
    }

    Log.trace("starting ble service and advertisements\n");
    pService->start();
    pServer->getAdvertising()->start();

    globalStatus.isBleInitialized = true;

    Log.trace("listening on bluetooth as [%s]\n", globalStatus.deviceName.c_str());
  }
}
