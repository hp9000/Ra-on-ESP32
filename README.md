# Ra Radiosonde Receiver - Firmware for ESP32 

This is a ported firmware for a Ra Radiosonde Receiver running on ESP32 HW (like TTGO Board or Heltec LoRa32 V3).
As the named boards have a less capable RX chip, fewer sonde types than the original Ra HW can be decoded.

This is for the **TTGO** board. If you have a Heltec LoRa32 V3 board switch to the Heltec branch. 

## Installation for the TTGO board

For installation you can run the following command.
Replace `<filename.bin>` by the path to the downloaded .bin file, replace `<port>` by your port. 

```
esptool.exe --chip esp32 --port <port> --baud 921600  --before default-reset --after hard-reset write-flash  -z --flash-mode keep --flash-freq keep --flash-size keep 0x0 <filename.bin>
```
