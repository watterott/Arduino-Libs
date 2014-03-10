# LCD-BackPack

## SerLCD Firmware

All standard characters received over the serial port will be shown on the display.
**Special characters:**
0x0D (CR) -> new line, 
0x0A (LF) -> new line, 
0x08 (Backspace) -> deleting last character

### Reset
The firmware needs about 2s to start because of the Arduino bootloader.
If 0x12 (18) is sent after this time and within 500ms then the baudrate will be set to 9600.

### Commands
* Display Commands
    * Start Byte: 0xFE (254)
    * Data Byte (HD44780 compatible commands)
        * 0x01 - clear display
        * 0x08 - display off
        * 0x0C - display on + cursor off
        * 0x0D - display on + blinking cursor on
        * 0x0E - display on + cursor on
        * 0x10 - move cursor left
        * 0x14 - move cursor right
        * 0x18 - scroll left
        * 0x1C - scroll right
        * 0x80... - set cursor position
* Special Commands
    * Start Byte: 0x7C (124)
    * Data Byte
        * 0x01 - backlight on (100%)
        * 0x02 - backlight off (0%)
        * 0x03 - 20 characters
        * 0x04 - 16 characters
        * 0x05 - 4 lines
        * 0x06 - 2 lines
        * 0x07 - 1 lines
        * 0x08 - re-init display
        * 0x09 - splash screen on/off
        * 0x0A - save lines 1+2 for custom splash screen
        * 0x0B - baudrate:   2400
        * 0x0C - baudrate:   4800
        * 0x0D - baudrate:   9600
        * 0x0E - baudrate:  14400
        * 0x0F - baudrate:  19200
        * 0x10 - baudrate:  38400
        * 0x11 - baudrate:  57600
        * 0x12 - baudrate: 115200
        * 0x13 - baudrate:  28800
        * 0x15 - contrast 0-100%
            * Parameter Byte: 0x00...0x64
        * 0x16 - RGB backlight on/off
        * 0x17 - backlight 0-100% (1 byte and if RGB is on then 3 bytes)
            * Parameter Byte(s): 0x00...0x64
        * 0x80...0x9F - backlight 0-100%
            * 0x80 -> 0%
            * 0x8C -> 40%
            * 0x96 -> 73%
            * 0x9D -> 100%
            * 0x9E -> 100%
            * 0x9F -> 100%
