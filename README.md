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

## Status
<table>
    <tr>
    <td></td>  <td></td> <td>coded</td> <td>air test</td>  <td>desk test</td> 
  </tr>
  <tr>
    <td rowspan="2">Vaisala</td>
    <td>RS41</td> <td>✔️</td> <td>✔️</td> <td>✔️</td> 
  </tr>
  <tr>
    <td>RS92</td> <td>❌</td> <td>❌</td> <td>❌</td> 
  </tr>
    <tr>
    <td rowspan="3">GRAW</td>
    <td>DFM17</td> <td>✔️</td> <td>✔️</td> <td>✔️</td>
  </tr>
  <tr>
    <td>DFM09</td> <td>✔️</td> <td>❌</td> <td>✔️</td> 
  </tr>
  <tr>
    <td>PS15</td> <td>✔️</td> <td>❌</td> <td>✔️</td> 
  </tr>
  <tr>
    <td rowspan="3">MODEM</td>
    <td>M10</td> <td>✔️</td> <td>❌</td> <td>✔️</td> 
  </tr>
  <tr>
    <td>M20</td> <td>✔️</td> <td>❌</td> <td>❌</td> 
  </tr>
    <tr>
    <td>Pilot</td> <td>❌</td> <td>❌</td> <td>❌</td> 
  </tr>
</table>
    
## License

This project is licensed under the BSD License - see the [LICENSE](LICENSE) file for details
