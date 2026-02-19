# Ra Radiosonde Receiver - Firmware for ESP32 

This is a ported firmware for a Ra Radiosonde Receiver running on ESP32 HW (like TTGO Board or Heltec LoRa32 V3).
As the named boards have a less capable RX chip, fewer sonde types than the original Ra HW can be decoded.

Precompiled binaries you will find by clicking on **Releases** on the right.

For installation use https://www.espboards.dev/tools/program/ or [esptool](docs) 

## Status
<table>
    <tr>
    <td></td>  <td></td> <td>coded</td> <td>air test</td>  <td>desk test</td> 
  </tr>
  <tr>
    <td>Vaisala</td><td>RS41</td> <td>✔️</td> <td>✔️</td> <td>✔️</td> 
  </tr>
    <tr>
    <td rowspan="3">GRAW</td>
    <td>DFM17</td> <td>✔️</td> <td>✔️</td> <td>✔️</td>
  </tr>
  <tr>
    <td>DFM09</td> <td>✔️</td> <td></td> <td>✔️</td> 
  </tr>
  <tr>
    <td>PS15</td> <td>✔️</td> <td></td> <td>✔️</td> 
  </tr>
  <tr>
    <td rowspan="2">MODEM</td>
    <td>M10</td> <td>✔️</td> <td></td> <td>✔️</td> 
  </tr>
  <tr>
    <td>M20</td> <td>❌</td> <td></td> <td></td> 
  </tr>
   <tr>
    <td rowspan="2">MEISEI</td>
    <td>iMS-100</td> <td>✔️</td> <td></td> <td>✔️</td> 
  </tr>
  <tr>
    <td>RS11G</td> <td>✔️</td> <td></td> <td></td> 
  </tr>
   <tr>
    <td rowspan="2">NEWSKY</td>
    <td>CF-06-AH</td> <td>✔️</td> <td></td> <td>✔️</td> 
  </tr>
  <tr>
    <td>GTH3</td> <td>✔️</td> <td></td> <td></td> 
  </tr>
</table>

## Acknowledgements
A big Thanks! for the support of this project to Demy, Dennis, DF9DQ and DL5RCB!
    
## License

This project is licensed under the BSD License - see the [LICENSE](LICENSE) file for details
