#include "ble.h"
#include <BLEDevice.h>
#include <BLE2902.h>
#include <Arduino.h>
#include "lpclib_types.h"
#include "lilygo.h"

/* keep in sync with sys.c !! */
#define SYS_OPCODE_BLE_MESSAGE  3  
#define COMMAND_LINE_SIZE   400
#define MAX_BLE_MESSAGES    3
typedef struct {
    uint8_t opcode;
    union {
        LPCLIB_Event event;
        int bufferIndex;
    };
} publicSYS_Message;
/* keep in sync with sys.c !! */

char bleCommandLine[MAX_BLE_MESSAGES][COMMAND_LINE_SIZE];
BLECharacteristic *pRxCharacteristic;
BLECharacteristic *pTxCharacteristic;
QueueHandle_t msgQueue = nullptr;

void BLE_setMsgQueue(QueueHandle_t q) { 
    msgQueue = q; 
}

void BLE_setup(bool isSpecialDevice)
{
  BLEDevice::init("RaOnTTGO");
  
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pTxCharacteristic =
    pService->createCharacteristic(UUID_VSP_CHAR_TX, BLECharacteristic::PROPERTY_WRITE_NR);
  pTxCharacteristic->setCallbacks(new MyCallbacks());

  pRxCharacteristic =
    pService->createCharacteristic(UUID_VSP_CHAR_RX, BLECharacteristic::PROPERTY_NOTIFY);
  pRxCharacteristic->addDescriptor(new BLE2902());
  pRxCharacteristic->setValue("xy");
  
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void MyCallbacks::onWrite (BLECharacteristic *pCharacteristic) 
{ 
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    publicSYS_Message aMessage;
    static int nextBufferIdx = 0;

    strncpy(bleCommandLine[nextBufferIdx], pCharacteristic->getValue().c_str(), COMMAND_LINE_SIZE);
    aMessage.bufferIndex = nextBufferIdx; 
    nextBufferIdx   = ++nextBufferIdx % MAX_BLE_MESSAGES;
    aMessage.opcode = SYS_OPCODE_BLE_MESSAGE;    
    xQueueSendFromISR(msgQueue, &aMessage,&xHigherPriorityTaskWoken );  
    if( xHigherPriorityTaskWoken == pdTRUE ){portYIELD_FROM_ISR(); }
}