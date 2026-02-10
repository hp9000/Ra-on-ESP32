# Ra Radiosonde Receiver - Firmware for ESP32 

This is a ported firmware for a Ra Radiosonde Receiver running on ESP32 HW (like TTGO Board or Heltec LoRa32 V3).
As the named boards have a less capable RX chip, fewer sonde types than the original Ra HW can be decoded.
Currently both boards can decode **RS41** and **DFM** sondes.

## Installation for the TTGO board
Click on **Releases** on the right and download the latest FW 'Ra on TTGO VX.Y'
For installation run the following command. Replace `<filename.bin>` by the path to the downloaded .bin file, replace `<port>` by your port. 

```
esptool.exe --chip esp32 --port <port> --baud 921600 write_flash 0x0 <filename.bin>
```

## Installation for the Heltec V3 Board

Click on **Releases** on the right and download the latest FW 'Ra on HeltecV3 VX.Y'
For installation run the following command. Replace `<filename.bin>` by the path to the downloaded .bin file, replace `<port>` by your port. 

```
esptool --chip esp32s3 --port <port> --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x0 <filename.bin>
```
Running on battery the Heltec board is sometimes resetting. You can avoid this by using a 'big' battery (or a Powerbank on USB). I could not find the problem in SW, so I think, it is a HW problem. 
For this board there will be no more versions for more sondes. 

## Known Bugs

v0.5: 
  - TTGO board goes to sleep mode, if app is in scanner screen for more than 30 sec
    
## License

This project is licensed under the BSD License - see the [LICENSE](LICENSE) file for details
