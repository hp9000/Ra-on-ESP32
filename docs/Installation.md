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