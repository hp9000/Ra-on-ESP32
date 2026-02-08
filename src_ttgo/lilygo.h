#ifndef __LILYGO_H
#define __LILYGO_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h" 
#include "freertos/queue.h"    
#include <ble.h>
#include "espPorting.h"

enum {
    SCREEN_CURRENT,
    SCREEN_STARTUP,
    SCREEN_SONDEDATA,
    SCREEN_DEBUG
};

enum {
  BT_STATE_DISCONNECTED,
  BT_STATE_CONNECTED,
};



class LilyGo {
public:
    LilyGo();
    void setup();
    void a100msTask();
    void setMsgQueue(QueueHandle_t q); 
    uint32_t getSerialNo(); 
    float getBatVoltage();
    void setBtState(bool);

    void SX1278_setup();
    void SX1278_ioctl(const SX1278_Config config[]);
    void SX1278_setBitRate(uint16_t bitrate);
    void SX1278_setRadioFrequency(float freqInHz);
    void SX1278_readRSSI(float* newLevel);
    void EEPROM_setup();
    void EEPROM_writeCfg(uint32_t frequency);
    void EEPROM_writeCfg(uint8_t detector);
    uint32_t EEPROM_getFrequency() {return frequencyInEeprom;};
    uint8_t  EEPROM_getDetector()  {return detectorInEeprom;};
    void setDisplayData(double, double, double, float ,char* , float, uint32_t);
    void setDisplayFreq(float);
    void toggleDebugScreen();
    void setDebugCrc(int eCrcCntr, int blockCntr);

private:
    void OLED_setup();
    void OLED_drawBat();
    void OLED_drawRSSI();
    void OLED_show(bool);
    void OLED_updateGraphics(uint8_t displayCmd);
    void OLED_updateVoltage(float vBatt);
    void OLED_updateScreen(uint8_t screen = SCREEN_CURRENT);
    void handleConsole(const char *cmd);

    BLECharacteristic *pRxCharacteristic;
    int guiCmdIdx = 0, initScreenCntDwn = 0, displayAgeCntdwn = 0, 
        debug_age = 0, debug_RS41frameNr = 0, debug_RS41CrcCntr = 0, debug_RS41BlockCntr = 0;
    bool BTisConnected;
    uint8_t activeScreen = SCREEN_STARTUP, detectorInEeprom;
    uint32_t latestDebugMsg,frequencyInEeprom,SerialNoEsp,versionBT = 0x05000000;
    double lat,lon,alt;
    float vBatt,freqMhz,rssi;
    char id[20],guiCmd[20];
    // Reboot reason messages; displayed as numbers in 'Bootloader' field on App screen after a reset 
    const char *rereMsg[16] = { "UNKNOWN", "POWERON", "EXTERNAL PIN", "SW RESTART", "PANIC", "INTERRUPT WTD",
                                "TASK WTD", "OTHER WTD", "EXITDEEPSLEEP", "BROWNOUT", "SDIO", "by USB",
                                "by JTAG", "EFUSE ERROR", "POWER GLITCH", "CPU_LOCKUP" };
};


#endif // __LILYGO_H
