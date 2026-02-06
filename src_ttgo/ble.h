#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <BLEUtils.h>
#include <BLEDevice.h>
#include <ble.h>
#include "lilygo.h"
#include "bridge.h"

#define SERVICE_UUID      "569a1101-b87f-490c-92cb-11ba5ea5167c"
#define UUID_VSP_CHAR_RX  "569a2000-b87f-490c-92cb-11ba5ea5167c"
#define UUID_VSP_CHAR_TX  "569a2001-b87f-490c-92cb-11ba5ea5167c"
#define UUID_VSP_CHAR_CTS "569a2002-b87f-490c-92cb-11ba5ea5167c"
#define UUID_VSP_CHAR_RTS "569a2003-b87f-490c-92cb-11ba5ea5167c"

void BLE_setup(bool);
void BLE_setMsgQueue(QueueHandle_t q);

class MyCallbacks: public BLECharacteristicCallbacks 
{
//   void onRead  (BLECharacteristic *pCharacteristic) {  print("callback onRead");}  
//   void onStatus(BLECharacteristic *pCharacteristic) {  print("callback onStatus");}  
//   void onNotify(BLECharacteristic *pCharacteristic) {  print("callback onNotify");}  
  void onWrite (BLECharacteristic *pCharacteristic); 
};

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {

  void onConnect(BLEServer* pServer) {
     ttgo_setBtState(true);
     //ESP_LOGE("HP","BT_STATE_CONNECT");
  };

  void onDisconnect(BLEServer* pServer) {
    ttgo_setBtState(false);
    //ESP_LOGE("HP","BT_STATE_DISCONNECT");
    BLEDevice::stopAdvertising();
    pServer->getAdvertising()->start();
  }
};

#endif