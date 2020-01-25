| Signal        | Pin | GPIO | Comment                                 |
| ------------- | --- | ---- | --------------------------------------- |
| RST/Reset     | D3  | 0    | When held high, then we reset the RC522 |
| SPI CS/SS/SDA | D8  | 15   | The Slave Select                        |
| SPI MOSI      | D7  | 13   | The MOSI data                           |
| SPI MISO      | D6  | 12   | The MISO data                           |
| SPI SCK       | D5  | 14   | The clock                               |

sda,sck,mosi,miso,...,gnd,rst,3.3v
