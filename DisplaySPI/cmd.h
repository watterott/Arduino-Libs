#ifndef _CMD_H_
#define _CMD_H_


/*
  PARAMTER    - BIT SIZE
  color       - 16 bit
  x,y,z,w,h,r - 8 bit for screens < 255x255 pixel, otherwise 16 bit
  other       - 8 bit
*/
enum _COMMANDS
{
  //general commands
  CMD_NOP1 = 0xFF,           //NOP / poll
  CMD_NOP2 = 0x00,           //NOP / poll
  CMD_VERSION,               //get firmware version (4bytes "x.xx")
  CMD_TEST,                  //start test program
  CMD_STATUS,                //get status (1byte)
  CMD_FEATURES,              //get available features (_CMDFEATURES)
  CMD_CTRL,                  //control options (_CTRLOPTIONS)
  CMD_PIN,                   //pin mode/config
  CMD_ADC,                   //read ADC pin (1byte channel), returns 16bit -> 2byte

  //display commands
  CMD_LCD_LED = 0x10,        //set backlight: power (1byte 0-100)
  CMD_LCD_RESET,             //reset display
  CMD_LCD_POWER,             //display panel power: power (1byte 0=off, 1=on)
  CMD_LCD_RAWCMD,            //send raw command to display (1byte)
  CMD_LCD_RAWDAT,            //send raw data to display (1byte)

  CMD_LCD_ORIENTATION = 0x20,//orientation (1byte 0=0 9=90 18=180 27=270)
  CMD_LCD_WIDTH,             //get display width (always 16bit -> 2byte)
  CMD_LCD_HEIGHT,            //get display height (always 16bit -> 2byte)
  CMD_LCD_INVERT,            //invert (1byte 0=off, 1=on)
  CMD_LCD_FGCOLOR,           //set foreground color (2byte: RGB565)
  CMD_LCD_BGCOLOR,           //set background color (2byte: RGB565)
  CMD_LCD_TERMINAL,          //start terminal mode
  CMD_LCD_DRAWIMAGE,         //draw image: x0, y0, w, h, color_mode, color...

  CMD_LCD_CLEAR,             //clear display: color
  CMD_LCD_CLEARFG,           //clear display (fg color)
  CMD_LCD_CLEARBG,           //clear display (bg color)

  CMD_LCD_DRAWPIXEL,         //draw pixel: x0, y0, color
  CMD_LCD_DRAWPIXELFG,       //draw pixel: x0, y0 (fg color)
  CMD_LCD_DRAWPIXELBG,       //draw pixel: x0, y0 (bg color)

  CMD_LCD_DRAWLINE,          //draw line: x0, y0, x1, y1, color
  CMD_LCD_DRAWLINEFG,        //draw line: x0, y0, x1, y1 (fg color)
  CMD_LCD_DRAWLINEBG,        //draw line: x0, y0, x1, y1 (bg color)

  CMD_LCD_DRAWLINES,         //draw lines: color, n, x0, y0...xn, yn
  CMD_LCD_DRAWLINESFG,       //draw lines: n, x0, y0...xn, yn (fg color)
  CMD_LCD_DRAWLINESBG,       //draw lines: n, x0, y0...xn, yn (bg color)

  CMD_LCD_DRAWRECT,          //draw rectangle: x0, y0, w, h, color
  CMD_LCD_DRAWRECTFG,        //draw rectangle: x0, y0, w, h (fg color)
  CMD_LCD_DRAWRECTBG,        //draw rectangle: x0, y0, w, h (bg color)

  CMD_LCD_FILLRECT,          //fill rectangle: x0, y0, w, h, color
  CMD_LCD_FILLRECTFG,        //fill rectangle: x0, y0, w, h (fg color)
  CMD_LCD_FILLRECTBG,        //fill rectangle: x0, y0, w, h (bg color)

  CMD_LCD_DRAWRNDRECT,       //draw round rectangle: x0, y0, w, h, radius, color
  CMD_LCD_DRAWRNDRECTFG,     //draw round rectangle: x0, y0, w, h, radius (fg color)
  CMD_LCD_DRAWRNDRECTBG,     //draw round rectangle: x0, y0, w, h, radius (bg color)

  CMD_LCD_FILLRNDRECT,       //fill round rectangle: x0, y0, w, h, radius, color
  CMD_LCD_FILLRNDRECTFG,     //fill round rectangle: x0, y0, w, h, radius (fg color)
  CMD_LCD_FILLRNDRECTBG,     //fill round rectangle: x0, y0, w, h, radius (bg color)

  CMD_LCD_DRAWCIRCLE,        //draw circle: x0, y0, radius, color
  CMD_LCD_DRAWCIRCLEFG,      //draw circle: x0, y0, radius (fg color)
  CMD_LCD_DRAWCIRCLEBG,      //draw circle: x0, y0, radius (bg color)

  CMD_LCD_FILLCIRCLE,        //fill circle: x0, y0, radius, color
  CMD_LCD_FILLCIRCLEFG,      //fill circle: x0, y0, radius (fg color)
  CMD_LCD_FILLCIRCLEBG,      //fill circle: x0, y0, radius (bg color)

  CMD_LCD_DRAWELLIPSE,       //draw ellipse: x0, y0, radius_x, radius_y, color
  CMD_LCD_DRAWELLIPSEFG,     //draw ellipse: x0, y0, radius_x, radius_y (fg color)
  CMD_LCD_DRAWELLIPSEBG,     //draw ellipse: x0, y0, radius_x, radius_y (bg color)

  CMD_LCD_FILLELLIPSE,       //fill ellipse: x0, y0, radius_x, radius_y, color
  CMD_LCD_FILLELLIPSEFG,     //fill ellipse: x0, y0, radius_x, radius_y (fg color)
  CMD_LCD_FILLELLIPSEBG,     //fill ellipse: x0, y0, radius_x, radius_y (bg color)

  CMD_LCD_DRAWTEXT,          //draw text: fg_color, bg_color, x0, y0, size_clear, length, text
  CMD_LCD_DRAWTEXTFG,        //draw text: x0, y0, size_clear, length, text (fg color)
  CMD_LCD_DRAWTEXTBG,        //draw text: x0, y0, size_clear, length, text (bg color)

  CMD_LCD_DRAWSTRING,        //draw string: fg_color, bg_color, x0, y0, size_clear, text - end with 0x00
  CMD_LCD_DRAWSTRINGFG,      //draw string: x0, y0, size_clear, text - end with 0x00 (fg color)
  CMD_LCD_DRAWSTRINGBG,      //draw string: x0, y0, size_clear, text - end with 0x00 (bg color)

//  CMD_LCD_USERIMG,           //user image: size 4x4 8x8 16x16 32x32, color mode
//  CMD_LCD_DRAWUSERIMG,       //draw user image

  //touch panel commands
  CMD_TP_POS = 0x80,         //get last position and pressure (x, y, z)
  CMD_TP_X,                  //get last x position
  CMD_TP_Y,                  //get last y position
  CMD_TP_Z,                  //get current pressure
  CMD_TP_WAITPRESS,          //wait till press and get position (x, y) after press
  CMD_TP_WAITRELEASE,        //wait till release and get position (x, y) after release
  CMD_TP_WAITMOVE,           //wait till move and get direction (1byte: 0x01=x-, 0x02=x+, 0x04=y-, 0x08=y+) after move
  CMD_TP_CALIBRATE,          //calibrate touch panel (will not be saved to flash)

  //rotary encoder commands
  CMD_ENC_POS = 0x90,        //get encoder pos + switch
  CMD_ENC_SW,                //get switch state (0x01=press, 0x02=long-press)
  CMD_ENC_WAITPRESS,         //wait till press
  CMD_ENC_WAITRELEASE,       //wait till release

  //navigation switch commands
  CMD_NAV_POS = 0xA0,        //get nav pos + switch
  CMD_NAV_SW,                //get switch state (0x01=press, 0x02=long-press, 0x10=right, 0x20=left, 0x40=up, 0x80=down)
  CMD_NAV_WAITPRESS,         //wait till press
  CMD_NAV_WAITRELEASE,       //wait till release
};


enum _CMDFEATURES
{
  FEATURE_LCD = 0x01, //display
  FEATURE_TP  = 0x02, //touch panel
  FEATURE_ENC = 0x04, //rotary encoder
  FEATURE_NAV = 0x08, //navigation switch
  FEATURE_LDR = 0x10, //light sensor (LDR)
  FEATURE_IRQ = 0x20, //IRQ output
};


enum _INTERFACES
{
  INTERFACE_UART  = 0x01,
  INTERFACE_I2C   = 0x02,
  INTERFACE_SPI   = 0x04,
  INTERFACE_RESET = 0xFF,
};


enum _CTRLOPTIONS
{
  CMD_CTRL_SAVE = 0x01, //save current settings to flash
  CMD_CTRL_INTERFACE,   //interface (1byte)
  CMD_CTRL_BAUDRATE,    //UART baud rate (4byte = 32bit)
  CMD_CTRL_ADDRESS,     //I2C address (1byte)
  CMD_CTRL_BYTEORDER,   //Byte order (1byte, 0=big, 1=little)
  CMD_CTRL_SYSCLOCK,    //in MHz (1byte)
  CMD_CTRL_FEATURES,    //enable/disable features
};


enum _COLORMODES
{
  COLOR_BW     =    1, // 1 bit black/white   (RLE: x=1byte, m=0xAA)
  COLOR_WB,            // 1 bit white/black   (RLE: x=1byte, m=0xAA)
  COLOR_FG,            // 1 bit fg/bg         (RLE: x=1byte, m=0xAA)
  COLOR_BG,            // 1 bit bg/fg         (RLE: x=1byte, m=0xAA)
  COLOR_RGB323 =    8, // 8 bit [  8   4   8] (RLE: x=1byte, m=0xAA)
  COLOR_RGB332,        // 8 bit [  8   8   4] (RLE: x=1byte, m=0xAA)
  COLOR_RGB233,        // 8 bit [  4   8   8] (RLE: x=1byte, m=0xAA)
  COLOR_GRAY,          // 8 bit gray scale    (RLE: x=1byte, m=0xAA)
  COLOR_RGB565 =   16, //16 bit [ 32  64  32] (RLE: x=2byte, m=0xAAAA)
  COLOR_RGB888 =   24, //24 bit [256 256 256] (RLE: x=3byte, m=0xAA)
  COLOR_RLE    = 0x80, //use RLE with magic number (x,x,m,n,x...0,0) n=1byte (n=0 for stop)
};


#endif //_CMD_H_
