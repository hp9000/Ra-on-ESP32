#include "bridge.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" 
#include "LILYGO.h" 
#include "CRC16.h"
#include "CRC.h"
//#include <string>

extern BLECharacteristic *pRxCharacteristic;
float freq,rssi;
double lat, lon, alt;
char id[10],type[10];

LilyGo myLilyGoBoard;
extern "C" {

    void ttgo_setMsgQueue(QueueHandle_t queue) {
       myLilyGoBoard.setMsgQueue(queue);
    }; 

    void ttgo_setup() {
        myLilyGoBoard.setup();
    };

    void ttgo_100msTask(){
        myLilyGoBoard.a100msTask();
    };

    void ttgo_setBtState(bool state) {
        myLilyGoBoard.setBtState(state);
    };

    uint32_t ttgo_getSerialNo() {
        return myLilyGoBoard.getSerialNo(); 
    };

    float ttgo_getBatVoltage() {
        return myLilyGoBoard.getBatVoltage();
    };

    void ttgo_setDisplayData(double lat, double lon, double alt, float freq, char *id, float rssi, uint32_t frameCounter)
    {
        myLilyGoBoard.setDisplayData(lat, lon, alt, freq, id, rssi, frameCounter);
    };

    void ttgo_toggleDebugScreen() {
        myLilyGoBoard.toggleDebugScreen();
    };

    void ttgo_toggleScannerScreen(int enable) {
        myLilyGoBoard.toggleScannerScreen(enable);
    };

    void ttgo_debug(int eCrcCntr, int blockCntr)
    {
        myLilyGoBoard.setDebugCrc(eCrcCntr, blockCntr);
    }

    void ttgo_setDisplayFreq(float freqHz)
    {
        myLilyGoBoard.setDisplayFreq(freqHz);
    };

    void ttgo_sendBtMessage( char* msg)
    {
    //    myLilyGoBoard.getInfosFromMsg(msg);
        // if(strncmp(msg,"#3,3",4) != 0)
        // {
        //     ESP_LOGE("HP","BT: %s",msg);
        // }

        if (pRxCharacteristic != nullptr) {
            int16_t l = strlen(msg);
            char *mPtr = &msg[0];
            do{
                if(l>20){
                    pRxCharacteristic->setValue((uint8_t*)mPtr,20);
                    mPtr+=20;
                    l-=20;
                }
                else{
                    pRxCharacteristic->setValue((uint8_t*)mPtr,l);
                    l=0;
                }
                pRxCharacteristic->notify();
                vTaskDelay(5/portTICK_PERIOD_MS); 
            }while(l>0);
        }
    };

    void SX1278_ioctl(const SX1278_Config config[]) {
        myLilyGoBoard.SX1278_ioctl(config);
    };

    void SX1278_setBitRate(uint16_t bitrate)
    {
        myLilyGoBoard.SX1278_setBitRate(bitrate);
    };

    float SX1278_setRadioFrequencyHz(uint32_t freqHz, bool readRssi)
    {
        return myLilyGoBoard.SX1278_setRadioFrequencyHz(freqHz, readRssi);
    };

    void SX1278_readRSSI(float* newLevel)
    {
        myLilyGoBoard.SX1278_readRSSI(newLevel);
    };

    void ttgo_writeFrequency2Eeprom(uint32_t frequency)
    {
        myLilyGoBoard.EEPROM_writeCfg(frequency);
    };  

    void ttgo_writeDetector2Eeprom(uint8_t detector)
    {
        myLilyGoBoard.EEPROM_writeCfg(detector);
    };

    uint32_t ttgo_getFrequency()
    {
        return myLilyGoBoard.EEPROM_getFrequency();
    };

    uint8_t ttgo_getDetector()
    {
        return myLilyGoBoard.EEPROM_getDetector();
    };

    uint16_t getCRC(const uint8_t* buffer, size_t length) 
    {
        CRC16 crc(CRC16_CCITT_FALSE_POLYNOME, CRC16_CCITT_FALSE_INITIAL);
        crc.add(buffer, length);
        return(crc.calc());
    }

    uint16_t getCRC2(const uint8_t* buffer, size_t length, uint16_t initialValue ) 
    {
        CRC16 crc(CRC16_CCITT_FALSE_POLYNOME, initialValue);
   //     crc.setInitial(initialValue);
        crc.add(buffer, length);
        return(crc.calc());
    }

    uint32_t __RBIT(uint32_t v) {  //esp32_rbit
    v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
    v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
    v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
    v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
    v = ( v >> 16             ) | ( v               << 16);
    return v;
    }
}