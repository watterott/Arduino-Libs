# mSD-Shield

## Schematics
The schematics can be found [here](https://github.com/watterott/mSD-Shield#msd-shield).

## Examples
Examples can be found in the Arduino IDE under ```File -> Examples -> mSD_Shield``` and ```File -> Examples -> SD```.

## mSD-Shield + Ethernet-Shield
If using the Ethernet-Shield together with the mSD-Shield, this must be initialized before using the SD-Card. Because otherwise the W5100 Ethernet controller blocks the SPI interface.

## mSD-Shield v2
The mSD-Shield v2 uses the ICSP connector for the SPI signals and so all Arduino boards can be used.

## mSD-Shield v1 (not mSD-Shield Mega-Edition)
For Hardware-SPI support on Mega boards connect the mSD-Shield v1 as follows.
No Software changes are required.

         Mega   mSD-Shield v1
    SCK   52   ->   13
    MOSI  51   ->   11
    MISO  50   ->   12
    
For using the RTC on Mega boards the I2C pins have to be changed.

        Mega   mSD-Shield v1
    SDA  20   ->   A4
    SCL  21   ->   A5
