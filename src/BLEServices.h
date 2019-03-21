#ifndef BLEServices_h
#define BLEServices_h

#include <WString.h>

#include <BLECharacteristic.h>

struct BLECharacteristicConf 
{
  String uuid;
  uint32_t properties; 
  BLECharacteristicCallbacks *callbacks;
};

void initCharacteristic(BLEService *pService, String uuid, uint32_t properties, BLECharacteristicCallbacks *callbacks);

void startBLE(String serviceUuid, BLECharacteristicConf confs[], int confsSize);

#endif