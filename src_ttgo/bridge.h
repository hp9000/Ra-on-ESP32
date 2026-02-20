#ifndef BRIDGE_H
#define BRIDGE_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" 
#include "espPorting.h"

#ifdef __cplusplus
extern "C" {
#endif

void ttgo_setup();
void ttgo_100msTask();
void ttgo_setMsgQueue(QueueHandle_t queue);
void ttgo_setBtState(bool state);
uint32_t ttgo_getSerialNo();
float ttgo_getBatVoltage();
void ttgo_sendBtMessage( char* msg);
void ttgo_writeFrequency2Eeprom(uint32_t frequency);
void ttgo_writeDetector2Eeprom(uint8_t detector);
uint32_t ttgo_getFrequency();
//float ttgo_setSysFrequency(uint32_t FreqHz, bool readRssi);
uint8_t ttgo_getDetector();
void ttgo_setDisplayData(double, double, double, float ,char* , float, uint32_t);
void ttgo_setDisplayFreq(float);
void ttgo_toggleDebugScreen();
void ttgo_toggleScannerScreen(int enable);
void ttgo_debug(int,int);

void SX1278_ioctl(const SX1278_Config config[]);
void SX1278_setBitRate(uint16_t bitrate);
float SX1278_setRadioFrequencyHz(uint32_t freqHz, bool readRssi);
void SX1278_readRSSI(float* newLevel);

uint16_t getCRC(const uint8_t* buffer, size_t length);
uint16_t getCRC2(const uint8_t* buffer, size_t length, uint16_t initialValue );

#ifdef __cplusplus
}
#endif

#endif