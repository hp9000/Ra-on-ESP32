#include <lilygo.h>
#include "freertos/stream_buffer.h"
#include "bridge.h"
#include "scanner.h"
#include "sys.h"

#define PIN_DIO1 33
#define PIN_DIO2 32

extern "C" {
    void PIN_INT3_IRQHandler2(unsigned int bit);
}

SYS_Handle sys;
SCANNER_Handle scanner;
SONDE_Handle sonde;

TaskHandle_t xTaskScanner = NULL;
TaskHandle_t xTaskSyncDet = NULL;
StreamBufferHandle_t xBitBuffer;
const size_t xStreamBufferSizeBytes = 1024;
const size_t xTriggerLevel = 1; 

int i_cntr = 0,*hc=0;
IRAM_ATTR void onDIO1Edge() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t bit = (GPIO.in1.val /*>> (PIN_DIO2 - 32)*/) & 0x01;
//    xTaskNotifyFromISR(xTaskSyncDet, bit, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    xStreamBufferSendFromISR(xBitBuffer, &bit, 1, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) { portYIELD_FROM_ISR(); }
}

void SYNCDET_thread (void *param)
{ 
    uint8_t receivedBit;

    while (1) 
    {
        if (xStreamBufferReceive(xBitBuffer, &receivedBit, 1, portMAX_DELAY) > 0) {
            do
            {
                PIN_INT3_IRQHandler2(receivedBit);  // Call the IRQ handler to process the bit
            } while (xStreamBufferReceive(xBitBuffer, &receivedBit, 1, 0) > 0);      
        }
    }
} 

void setup() {
   Serial.begin(115200);

   ttgo_setup();
   SYS_open(&sys);
   SCANNER_open(&scanner);
   SONDE_open(&sonde);

   xBitBuffer = xStreamBufferCreate(xStreamBufferSizeBytes, xTriggerLevel);
   xTaskCreate(SYNCDET_thread, "SyncDet",  2000, (void *)sys,     40, &xTaskSyncDet);
   xTaskCreate(SYS_thread,     "System",  50000, (void *)sys,     30, NULL);
   xTaskCreate(SCANNER_thread, "Scanner", 20000, (void *)scanner, 20, &xTaskScanner);

   pinMode(PIN_DIO1, INPUT);   
   pinMode(PIN_DIO2, INPUT);
   attachInterrupt(PIN_DIO1, onDIO1Edge, RISING);
  }


void loop() 
{
  vTaskDelay(100/portTICK_PERIOD_MS);
  ttgo_100msTask();
}


