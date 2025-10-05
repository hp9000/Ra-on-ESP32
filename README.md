# Ra Radiosonde Receiver - Firmware for the Heltec V3 Board

This is a ported firmware for a Ra Radiosonde Receiver running on the **Heltec LoRa32 V3** board.

For a project overview see [Ra Hardware](https://github.com/einergehtnochrein/ra-hardware)

## Installation

For installation you can run the following command.
Replace `<filename.bin>` by the path to the downloaded .bin file, replace `<port>` by your port. 

```
esptool --chip esp32s3 --port <port> --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode keep --flash_freq keep --flash_size keep 0x0 <filename.bin>
```
