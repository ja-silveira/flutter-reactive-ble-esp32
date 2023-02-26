
/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-ffe9-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID2 "beb5483e-ffe4-4688-b7f5-ea07361b26a8"
uint32_t value = 0;
bool deviceConnected = false;
bool oldDeviceConnected = false;
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic2 = NULL;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("WT901BLE67");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic2 = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID2,
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
                                       
  pCharacteristic2->addDescriptor(new BLE2902());

  pCharacteristic->setValue("im ffe9");
  pCharacteristic->setValue("im ffe4");
  
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setAdvertisementType(ADV_TYPE_IND); // ADV_TYPE_IND

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x05);
  std::string strServiceData = "";
  strServiceData += (char)0x07;
  strServiceData += (char)0xFF;
  strServiceData += (char)0xE8;
  strServiceData += (char)0xCB;
  strServiceData += (char)0xED;
  strServiceData += (char)0x5C;
  strServiceData += (char)0x84;
  strServiceData += (char)0xD0;
  oAdvertisementData.addData(strServiceData);
  
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  if (deviceConnected) {
        byte bArr1[] = {0x55, 0x61, 0x21, 0x00,
        0x07, 0x00, 0x19, 0x08,
        0xff, 0xff, 0xfc, 0xff,
        0x20, 0x00, 0x1d, 0x00,
        0x65, 0xff, 0x74, 0x54};

        byte bArr2[] = {0x55, 0x61, 0xff, 0x00,
        0x07, 0x00, 0x19, 0x08,
        0xff, 0xaa, 0xfc, 0xff,
        0x20, 0x00, 0x1d, 0x00,
        0x65, 0xff, 0xda, 0x54};

        byte bArr3[] = {0x55, 0x61, 0x1f, 0x00,
        0x03, 0x00, 0x1a, 0x08,
        0x05, 0x50, 0xf2, 0xff,
        0x00, 0x00, 0x15, 0x00,
        0x64, 0xff, 0x83, 0x54};

        byte* arrs[3] = {bArr1, bArr2, bArr3};

        int index = random(0, 3);

        byte* selectedArray = arrs[index];

        pCharacteristic2->setValue(selectedArray, 20);
        pCharacteristic2->notify();
        value++;
        delay(500); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
