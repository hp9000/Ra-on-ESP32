#include <lilygo.h>
#include <sys.h>
#include <EEPROM.h>
#include <esp_mac.h>
#include <SPI.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "images.h"
#include "bridge.h"
#include "espPorting.h"
#include "esp_task_wdt.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

#define PIN_MISO 19
#define PIN_MOSI 27
#define PIN_SCK  5
#define PIN_CS   18
#define PIN_RST  23
#define SDA_OLED 21
#define SCL_OLED 22
#define PIN_BAT  35

#define SX127x_FREQUENCY_STEP_SIZE   61.03515625 // in Hz (32 MHz / 2^19)

SSD1306Wire  display(0x3c, SDA_OLED, SCL_OLED, GEOMETRY_128_64,I2C_TWO, 500000); 
static const char* TAG = "HP";
int taskCalled_Cntr = 0;

void handleConsole(const char *cmd);

static void screenSaverCallback(TimerHandle_t xTimer)    {
    display.displayOff();
}

LilyGo::LilyGo() {
    // Constructor implementation
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(14, OUTPUT);
    rssi = -128;
}

void LilyGo::setup() {
    uint8_t baseMac[6];
    
    BTisConnected = false;
    BLE_setup(true);   
    esp_base_mac_addr_get(baseMac);   // Bluetooth MAC: 48:CA:43:B5:87:6C
    SerialNoEsp = baseMac[3]<<16 |baseMac[4]<<8 |baseMac[5];
    EEPROM_setup();
    SX1278_setup();
    OLED_setup();
    analogReadResolution(12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_0db);
    adc1_config_width(ADC_WIDTH_12Bit);
    vBattOnStart = getBatVoltage();
    if(ESP_OK != esp_task_wdt_init(5,true)) {
        ESP_LOGE(TAG, "Failed to initialize task watchdog");
    }
    //ESP_LOGE("HP","LilyGo setup complete.");   
}   

void LilyGo::setMsgQueue(QueueHandle_t q) { 
    BLE_setMsgQueue(q);
}

void LilyGo::setBtState(bool state) {
    BTisConnected = state;

    display.setColor(BTisConnected ? WHITE : BLACK);          
    switch(activeScreen)
    {
        case SCREEN_STARTUP:
            display.setFont(ArialMT_Plain_16);
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawString(0, 37, "B  T");
            break;
        case SCREEN_SONDEDATA:
            if(BTisConnected){
              display.drawIco16x16(0,0, &BTon[0]);
            }
            else{
              display.fillRect(0,0,16,16);
            }
            break;
    }
    
    display.display();
    display.displayOn();
    display.setColor(WHITE);
    xTimerReset( screenSaverTimer, 0);
}

void LilyGo::OLED_setup(){
    OLED_drawScreen(SCREEN_STARTUP); 
 }


 void LilyGo::OLED_show(bool state){
    screenIsOff = !state;
    state ? display.displayOn() : display.displayOff();
 }


uint32_t LilyGo::getSerialNo() { 
    return SerialNoEsp;
}


float LilyGo::getBatVoltage()
{
    float vBattOld = vBatt;
    digitalWrite(14, HIGH);
    delay(1);
    vBatt = (analogRead(PIN_BAT) / 4095.0 * 2 * 3.3 * voltageCalibrationFactor); 
                       // voltage divider 100k/100k, ADC ref 3.3V, calibration;
    digitalWrite(14, LOW);
    if(vBatt > 4.17)   // Simple threshold to detect charging state, adjust as needed
        isCharging = true;  
    else if(vBatt > vBattOld + 0.01)
        isCharging = true;
    else if(vBatt < vBattOld)
        isCharging = false;
 
    OLED_updateVoltage(vBatt);

    return vBatt;
}

void LilyGo::EEPROM_setup() {
    EEPROM.begin(5);
    frequencyInEeprom = EEPROM.readLong(0);
    detectorInEeprom  = EEPROM.readByte(4);
    if((frequencyInEeprom < 400e6)||(frequencyInEeprom > 406e6)||detectorInEeprom > 15)  //SONDE_DETECTOR_LMS6
    {
        frequencyInEeprom = 405100000;
        detectorInEeprom  = 0;  // SONDE_DETECTOR_RS41_RS92
        EEPROM.writeLong(0,frequencyInEeprom);
        EEPROM.writeByte(4,detectorInEeprom);  
        EEPROM.commit();
        ESP_LOGE("HP", "Eeprom empty, wrote defaults");
    }
    else
    {
       //ESP_LOGE("HP", "Eeprom read f= %d, D= %d",frequencyInEeprom,detectorInEeprom);
       EEPROM.end();
    }
    freqMhz = frequencyInEeprom/1e6;
}

void LilyGo::EEPROM_writeCfg(uint32_t frequency)
{
    EEPROM.begin(5);
    EEPROM.writeLong(0, frequency);
    EEPROM.commit();
}

void LilyGo::EEPROM_writeCfg(uint8_t detector)
{
    EEPROM.begin(5);
    EEPROM.writeByte(4,detector);
    EEPROM.commit();
}

uint8_t sx1278ReadRegister(uint8_t reg) {
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(reg & 0x7F); // read command
  uint8_t value = SPI.transfer(0x00);
  digitalWrite(PIN_CS, HIGH);
  return value;
}

void sx1278WriteRegister0(uint8_t reg, uint8_t value) {
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(reg | 0x80); // write command
  SPI.transfer(value);
  digitalWrite(PIN_CS, HIGH);
}

void LilyGo::SX1278_readRSSI(float* newLevel)
{
    // Convert raw RSSI value to dBm 
    *newLevel = -sx1278ReadRegister(0x11) / 2.0f; 
}

void LilyGo::SX1278_setBitRate(uint16_t bitrate) {
    uint8_t value = (uint8_t)(32000000 / bitrate >> 8); // Assuming F_XOSC = 32 MHz
    sx1278WriteRegister0(0x02, value);  // RegBitrateMsb
    value = (uint8_t)(32000000 / bitrate & 0xFF);
    sx1278WriteRegister0(0x03, value);  // RegBitrateLsb
}

float LilyGo::SX1278_setRadioFrequencyHz(uint32_t freqInHz, bool needRssi) {
    float rssi = 0.0f;
    uint8_t spiBuff[32];
    int32_t freq = (uint32_t)(freqInHz/SX127x_FREQUENCY_STEP_SIZE);

    sx1278WriteRegister0(0x01, 0x01); // Standby mode, FSK
    delay(2);
    spiBuff[0] =  0x80 | 0x06;  //Opcode for set RF Frequencty
    spiBuff[3] = freq & 0xFF; freq >>= 8;
    spiBuff[2] = freq & 0xFF; freq >>= 8;
    spiBuff[1] = freq & 0xFF; 
    digitalWrite(PIN_CS, LOW);  //Enable radio chip-select
    SPI.transfer(spiBuff, 4);
    digitalWrite(PIN_CS, HIGH); //Disable radio chip-select  
    sx1278WriteRegister0(0x01, 0x04);   // FSRX mode
    delay(2);                           // TS_FS (standby->FSRX) = 60 us
    sx1278WriteRegister0(0x01, 0x05);   // RX mode
    delay(2);                           // TS_RE (FSRX   ->RX  ) < 1 ms

    if(needRssi) {
        vTaskDelay(1/portTICK_PERIOD_MS); // Wait for RSSI sample
        rssi = -sx1278ReadRegister(0x11) / 2.0f;
        updateTopSignals(freqInHz, rssi);
    }

    return rssi;    
}


void LilyGo::SX1278_setup() {
    pinMode(PIN_CS, OUTPUT);
    pinMode(PIN_RST, OUTPUT);

    digitalWrite(PIN_CS, HIGH); // Deselect the SX1278
    // SPI setup
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    // Reset the SX1278
    digitalWrite(PIN_RST, LOW);
    delay(100);
    digitalWrite(PIN_RST, HIGH);
    delay(100);

    screenSaverTimer = xTimerCreate( "SCREENSAVER-Timer",pdMS_TO_TICKS(60000), pdFALSE, (void *)NULL, screenSaverCallback);
    xTimerStart( screenSaverTimer, 0);

    // SX1278 general initialization 
    sx1278WriteRegister0(0x01, 0x01);       // FSK Standby Mode (LoRa aus, Mode = 001)
    sx1278WriteRegister0(0x0C, 0b00100011); // G1 = highest gain
    sx1278WriteRegister0(0x0D, 0b11111110); // RegRxConfig -> AFC & AGC, gain by AGC
    sx1278WriteRegister0(0x0E, 0b00000100); // RSSI Glättung (32 samples)
    sx1278WriteRegister0(0x14, 0x28);       // Bit-Synchronizer einschalten (optional für stabilere Daten)
    sx1278WriteRegister0(0x1E, 0b00000001); // RegAfcFei-> AFC Autoclear an, um Frequenzdrift der Sonde zu folgen 
    sx1278WriteRegister0(0x1F, 0xAA);       // Preamble Detektor On, 2 Bytes Sequenz
    sx1278WriteRegister0(0x30, 0x00);       // 
    sx1278WriteRegister0(0x31, 0x00);       // Continuous Mode aktivieren (PacketMode Bit 6 = 0)
    sx1278WriteRegister0(0x40, 0x00);       // DIO2 Mapping auf "Data" setzen
}

void LilyGo::SX1278_ioctl(const SX1278_Config config[]) {
    for (int i = 0; config[i].reg != 0xFF; i++) {
        uint8_t regAddr = config[i].reg;
        uint8_t targetValue = config[i].value;
        sx1278WriteRegister0(regAddr, targetValue);
    }
    delay(2); // Kurze Pause zur Stabilisierung (optional)
}       

void LilyGo::a100msTask()
{
    taskCalled_Cntr++;

    if (Serial.available()) {
        uint8_t key = Serial.read();
        if (key != 10) {
            guiCmd[guiCmdIdx++] = key;
        } else {
            guiCmd[guiCmdIdx] = 0;
            handleConsole(&guiCmd[0]);
            guiCmdIdx = 0;
        }
    }

    if (taskCalled_Cntr % 10 == 0){   // Every second, increase age of data  
            debug_age = (millis() - latestDebugMsg)/1000;
            if(activeScreen == SCREEN_DEBUG)
                OLED_drawScreen(SCREEN_DEBUG,false); 
   
            if(activeScreen == SCREEN_SCANNER)
               OLED_drawScreen(SCREEN_SCANNER,false);
    }

   
    if(taskCalled_Cntr == 30)         // After showing startup screen for 3 seconds, switch to main screen
            OLED_drawScreen(SCREEN_SONDEDATA); 

    if (taskCalled_Cntr % 100 == 0)   // Blink LED every 10 seconds to indicate the system is alive
        digitalWrite(LED_BUILTIN, HIGH);
    else if(taskCalled_Cntr % 100 == 1)
        digitalWrite(LED_BUILTIN, LOW);

}

void LilyGo::setDisplayData(double lat_in, double lon_in, double alt_in, float freq_in,char *id_in, float rssi_in, uint32_t frameCounter)
{
   xTimerStart(screenSaverTimer, 0);

   lat  = lat_in;
   lon  = lon_in;
   freqMhz = freq_in;
   alt  = alt_in;
   rssi = rssi_in;
   debug_RS41frameNr = frameCounter;
   strcpy(id,id_in);
   OLED_drawScreen(SCREEN_CURRENT,true);  
}

void LilyGo::setDisplayFreq(float freqHz)
{
   xTimerReset( screenSaverTimer, 0);

   freqMhz = freqHz/1e6;
   lat  = 0;
   lon  = 0;
   alt  = 0;
   rssi = -128.0f;
   debug_RS41frameNr = 0;
   id[0] = 0;
   OLED_drawScreen(SCREEN_CURRENT,true);
}


void LilyGo::OLED_drawScreen(uint8_t screen, bool disableScreenSaver)
{
    char s[40];
    if(disableScreenSaver){
        OLED_show(true);
        if(screen != SCREEN_SCANNER)
            xTimerReset( screenSaverTimer, 0);
    }

    if(screen != SCREEN_CURRENT)
        activeScreen = screen;  

    switch (activeScreen) {
        case SCREEN_STARTUP:
            display.init();
            display.flipScreenVertically();
            display.clear();
            display.displayOn();
            display.setColor(WHITE);
            display.drawXbm(0, 0, image_width, image_height, image_bits);
            display.setFont(ArialMT_Plain_24);
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.drawStringf(128,42,s,"V%d.%d",FIRMWARE_VERSION_MAJOR,FIRMWARE_VERSION_MINOR);
            break;
        case SCREEN_SONDEDATA:
            xTimerReset(screenSaverTimer, 0);
            display.clear();
            display.setColor(WHITE);
            display.setFont(ArialMT_Plain_16);
            display.setTextAlignment(TEXT_ALIGN_CENTER);
            display.drawStringf(63,0,s,"%6.3f",freqMhz);
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawStringf(0,32,s,"%7.5f",lat);
            display.drawStringf(0,48,s,"%7.5f",lon);
            display.drawStringf(0,16,s,"%s",id);
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.drawStringf(127,32,s,"%.0f  ",alt);
            display.setFont(ArialMT_Plain_10);
            display.drawString(127,37,"m");
            OLED_drawBat();
            OLED_drawRSSI();
            if(BTisConnected )
                display.drawIco16x16(0,0, &BTon[0]);

            break;
        case SCREEN_DEBUG: 
            char dbgMsg[20];
            display.clear();
            display.setColor(WHITE);
            display.setFont(ArialMT_Plain_16);
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            //display.drawString(0, 0,dbgMsg);
            if(debug_RS41BlockCntr > 0)
            {
                display.drawStringf(0,16,dbgMsg,"C:%d/%d",debug_RS41CrcCntr,debug_RS41BlockCntr);
                display.drawStringf(0,32,dbgMsg,"#%d",debug_RS41frameNr);
            }
            display.drawStringf(0,48,dbgMsg,"R:%s",rereMsg[esp_reset_reason()]);
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            if(debug_RS41BlockCntr > 0)
                display.drawStringf(127,16,dbgMsg,"%ds",debug_age);
            display.drawStringf(127,32,dbgMsg,"%.1fdB",rssi);
            break;
        case SCREEN_SCANNER: 
            display.clear();
            display.setColor(WHITE);
            display.setFont(ArialMT_Plain_16);    
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.drawStringf(0,0 ,s,"%6.2f",topSignals[0].freq/100.0);
            display.drawStringf(0,16,s,"%6.2f",topSignals[1].freq/100.0);
            display.drawStringf(0,32,s,"%6.2f",topSignals[2].freq/100.0);
            display.drawStringf(0,48,s,"%6.2f",topSignals[3].freq/100.0);
            display.setTextAlignment(TEXT_ALIGN_RIGHT);
            display.drawStringf(127,0 ,s,"%.1fdB",topSignals[0].rssi);
            display.drawStringf(127,16,s,"%.1fdB",topSignals[1].rssi);
            display.drawStringf(127,32,s,"%.1fdB",topSignals[2].rssi);
            display.drawStringf(127,48,s,"%.1fdB",topSignals[3].rssi);
            break;
        case SCREEN_SHUTDOWN:
            display.clear();
            display.setColor(WHITE);
            display.setFont(ArialMT_Plain_24);
            display.setTextAlignment(TEXT_ALIGN_CENTER);
            display.drawString(63,24,"Sleeping...");
            display.display();
            vTaskDelay(3000/portTICK_PERIOD_MS);
            display.displayOff();
            break;
        default:
            break;  
    }

    display.display();
}

void LilyGo::toggleDebugScreen()
{
    OLED_drawScreen((activeScreen == SCREEN_DEBUG) ? SCREEN_SONDEDATA : SCREEN_DEBUG, true);
}

void LilyGo::toggleScannerScreen(int enable)
{
    if(enable == 2){
        xTimerStop(screenSaverTimer, 0);
        OLED_drawScreen(SCREEN_SCANNER, true);
    }
    else{
        OLED_drawScreen(SCREEN_SONDEDATA, true);
        resetTopList();
    }
}

void LilyGo::switchOffScreen()
{
    OLED_show(false);
}

void LilyGo::setDebugCrc(int eCrcCntr, int blockCntr)
{
    xTimerReset( screenSaverTimer, 0);
    debug_RS41CrcCntr   = eCrcCntr;
    debug_RS41BlockCntr = blockCntr;
    latestDebugMsg  = millis();
    if(activeScreen == SCREEN_DEBUG)
        OLED_drawScreen(SCREEN_DEBUG, true);
}   

void LilyGo::OLED_updateVoltage(float vBatt_in)
{
    if(vBatt_in != vBattLast){
        vBatt     = vBatt_in;
        vBattLast = vBatt_in;
        if(activeScreen == SCREEN_SONDEDATA){
            OLED_drawBat();
            display.display();
        }   
    }
}

void LilyGo::OLED_drawBat()
{
//ESP_LOGE("HP","vBatt = %f", vBatt);
    //4.14 voll ohne laden, 3.0V leer, 3.9V ca. 50% Ladung
    //4.19 voll mit laden
    //3.8 nach 4h
    
    //display.drawProgressBar(104, 2, 24, 12, 50/*(uint8_t)(vBatt*100/4.2)*/);

    display.drawRect(104, 2, 24, 12);
    display.fillRect(102, 6, 2, 4);

    // if(isCharging)
    // {
    //     display.drawXbm(112, 3, 8, 10, bolt_tiny);
    // }
    // else
    {  
        int empty = (int)((4.0 - vBatt)*18.3);  
        if(empty < 0) 
          empty = 0;
        display.fillRect(106+empty, 4, 20-empty, 8);
    }
}

void LilyGo::OLED_drawRSSI()
{
//   uint8_t n;
//   if(rssi >= -65) n = 5;
//   else if((rssi < -65) && (rssi >= -80)) n = 4;
//   else if((rssi < -80) && (rssi >= -95)) n = 3;
//   else if((rssi < -95) && (rssi >= -110)) n = 2;
//   else n = 1;  
//   //   = (rssi+155)/27;
//   for(int i = 0; i < 5; i++)
//   {
//     if(i==n)
//       display.setColor(BLACK);
//     display.fillRect(103+i*5, 53, 4, 10);
//   }
//   display.setColor(WHITE);
    char s[20];
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_16);
    display.drawStringf(127,48,s,"%.0fdB",rssi);
}

void LilyGo::handleConsole(const char *cmd)
{
    //static uint8_t debug(1);
    //const char *onOffState[] = {"off","on"};
    static bool isDisplayOn(true);

    Serial.print("cmd> ");
    Serial.write(*cmd);
    Serial.println();
    switch(*cmd)
    {
      case 'h':
      {
          Serial.println("h:  help");    
//          Serial.println("lx:  log level x (0=none,1=error,2=warn,3=info,4=debug,5=verbose)");      
//          Serial.println("m:  toggle debug messages"); 
          Serial.println("d:  toggle Display");
          Serial.println("r:  register dump");
          Serial.println("x:  reboot");
          Serial.println("s:  Screen");
        //   ESP_LOGE("HP","t:  Takt (240,160,80,40,20,10)");
           
           
           //         ESP_LOGE("HP","w:  toggle WLAN");
          break;
      }      
      case 'd':
      {
          isDisplayOn = !isDisplayOn;
          OLED_show(isDisplayOn);
          break;
      }    
      case 'r':
      {
          for(int i=0;i<0x80;i++)
          {
             Serial.printf("Reg[0x%02x] = 0x%02x\n",i,sx1278ReadRegister(i));
          }
          break;
      } 
      case 'x':
      {
          esp_restart();
          break;
      } 
    //   case 'l':
    //   {
    //         esp_log_level_t level = (esp_log_level_t)(cmd[1]-48);
    //         esp_log_level_set("HP", level);
    //         Serial.printf("Log level set to %d\n",level);
    //         ESP_LOGD("HP","D:Log level set to %d\n",level);
    //         ESP_LOGE("HP","E:Log level set to %d\n",level);
    //         esp_log_write(ESP_LOG_DEBUG, "HP", "Test nativ: %d\n", level);
    //         break;
    // }


    //   case 'm':
    //   {
    //       char msg[40];
    //       debug = (debug + 1)%2;
    //       sprintf(msg,"debug messages %s", onOffState[debug]);
    //       ESP_LOGE("HP","%s",msg);
    //       break;
    //   }
      case 's':
      {        
          uint8_t snr = cmd[1]-48;
          if(snr < SCREEN_MAX)
          {
             OLED_drawScreen(snr,true);
          }
          break;
      }       
       
    //   case 't':
    //   {        
    //       uint8_t takt;
    //       sscanf(&cmd[1],"%d",&takt);
    //       ESP_LOGE("HP","Aktueller CPU-Takt: %d MHz",getCpuFrequencyMhz());
    //       if (setCpuFrequencyMhz(takt)) { // setCpuFrequencyMhz gibt true bei Erfolg zurück
    //          ESP_LOGE("HP","Taktänderung auf %d MHz erfolgreich!",takt);
    //       } else {
    //          ESP_LOGE("HP","Fehler: Taktänderung auf %d MHz fehlgeschlagen oder nicht unterstützt.",takt);
    //       }
    //       ESP_LOGE("HP","Aktueller CPU-Takt: %d MHz",getCpuFrequencyMhz());


    //     //   uint8_t snr = cmd[1]-48;
    //     //   if(snr < FINALSCREEN)
    //     //   {
    //     //     SYS_setInactivityTimeout(false);
    //     //     myLilyGoBoard.gotoNextScreen(cmd[1]-48);
    //     //   }
    //     //   uint8_t status=radio.getStatus();
    //     //   ESP_LOGE("HP","Status=%x",status);
    //     //   uint32_t ps   =radio.getPacketStatus();
    //     //   ESP_LOGE("HP","PkgStatus=%x",ps);
    //     //   uint16_t de   =radio.getDeviceErrors();
    //     //   ESP_LOGE("HP","DeviceErrors=%x",de);
    //       break;
    //   }  

      default:
        Serial.printf("key was %d\n",*cmd);
        break;  
    }
}


void LilyGo::updateTopSignals(uint32_t newFreq, float newRssi) {
    int existingIdx = -1;
    newFreq /= 10000; // Convert to 10 kHz steps for easier comparison and display

    // 1. Check if the frequency is already present in the top 4
    for (int i = 0; i < 4; i++) {
        if (topSignals[i].freq == newFreq) {
            existingIdx = i;
            break;
        }
    }

    if (existingIdx != -1) {
        // Frequency exists: Update RSSI only if the new signal is stronger
        if (newRssi > topSignals[existingIdx].rssi) {
            topSignals[existingIdx].rssi = newRssi;
        } else {
            return; // Existing entry is already stronger
        }
    } else {
        // New frequency: Only process if it's stronger than the current 4th place
        if (newRssi <= topSignals[3].rssi) return;
        
        // Replace the weakest signal (rank 4)
        topSignals[3].freq = newFreq;
        topSignals[3].rssi = newRssi;
    }

    // 2. Mini insertion sort to move the updated/new signal to its correct rank
    // Since only one element is out of order, max 3 swaps are needed
    for (int i = 3; i > 0; i--) {
        if (topSignals[i].rssi > topSignals[i-1].rssi) {
            Signal temp     = topSignals[i];
            topSignals[i]   = topSignals[i-1];
            topSignals[i-1] = temp;
        } else {
            break; // Correct position reached
        }
    }
}

void LilyGo::resetTopList() {
    for (int i = 0; i < 4; i++) {
        topSignals[i].freq = 0;
        topSignals[i].rssi = -128;
    }
}
