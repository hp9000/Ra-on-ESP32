# Ra Radiosonde Receiver - Firmware for ESP32 
A portable Radiosonde Receiver using the iRa App, for the price of a TTGO board

This is a ported firmware for a Ra Radiosonde Receiver running on ESP32 HW (like TTGO Board or Heltec LoRa32 V3).
The RX chip on this boards does not support AFSK demodulation, so some sonde types (IMET4, SRSC and others) cannot be decoded.

<img width="384" height="384" alt="image" src="https://github.com/user-attachments/assets/411f8b53-64cf-4430-b692-602dfc5b7f42" />

Precompiled binaries you will find by clicking on **Releases** on the right.

For installation use https://www.espboards.dev/tools/program/ or [esptool](docs) 

## Status (SX1278 board)
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
More sonde types might be implemented, if you send me HW samples or do the testing.

## Acknowledgements
A big Thanks! for the support of this project to Demy, Dennis, DF9DQ and DL5RCB!
    
## License

This project is licensed under the BSD License - see the [LICENSE](LICENSE) file for details
