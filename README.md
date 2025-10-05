# Ra Radiosonde Receiver - Firmware for the Heltec V3 Board

This is a ported firmware for a Ra Radiosonde Receiver running on ESP32 HW (like TTGO Board or Heltec LoRa32 V3). As the named boards have a less capable RX chip, fewer sonde types than the original Ra HW can be decoded.

This is for the **Heltec LoRa32 V3** board. If you have a TTGO board switch to the TTGO branch.

Running on battery the Heltec board is sometimes resetting. You can avoid this by using a 'big' battery (or a Powerbank on USB). I could not find the problem in SW, so I think, it is a HW problem. 
For this bord there will be no new versions for more sondes. Currently **RS41 and DFM** sondes can be decoded. 

## Installation

For installation you can run the following command.
Replace `<filename.bin>` by the path to the downloaded .bin file, replace `<port>` by your port. 

```
esptool --chip esp32s3 --port <port> --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x0 <filename.bin>
```
