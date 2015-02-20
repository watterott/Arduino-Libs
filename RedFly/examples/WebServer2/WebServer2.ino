/*
  Web Server with URL decoding/parameter support
  
  This sketch acts as a server using a RedFly-Shield. 
  For testing, open in your browser http://192.168.0.30/
  (Replace the IP with the one from your RedFly-Shield.)
 */

#include <RedFly.h>
#include <RedFlyServer.h>
#include <RedFlyNBNS.h>


#define FAVICONSIZE 318
const uint8_t FAVICON[FAVICONSIZE] PROGMEM = {
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x28, 0x01, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00,
	0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00,
	0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x80, 0x80,
	0x80, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF,
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x08, 0x66, 0x66, 0x80,
	0x00, 0x00, 0x00, 0x08, 0x66, 0x66, 0x66, 0x66, 0x80, 0x00, 0x00, 0x66,
	0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x08, 0x66, 0x66, 0x66, 0x66, 0x66,
	0x66, 0x80, 0x06, 0x7F, 0xFF, 0x76, 0x67, 0xFF, 0xF7, 0x60, 0x87, 0xF6,
	0x66, 0xF7, 0x7F, 0x66, 0x6F, 0x78, 0x6F, 0x66, 0x66, 0x6F, 0xF6, 0x6F,
	0x66, 0xF6, 0x6F, 0x6F, 0xFF, 0x6F, 0xF6, 0xFF, 0xF6, 0xF6, 0x6F, 0x66,
	0x66, 0x6F, 0xF6, 0x6F, 0x66, 0xF6, 0x67, 0xF6, 0x66, 0xF7, 0x7F, 0x66,
	0x6F, 0x76, 0x86, 0x7F, 0xFF, 0x76, 0x67, 0xFF, 0xF7, 0x68, 0x06, 0x66,
	0x66, 0x66, 0x66, 0x66, 0x66, 0x60, 0x08, 0x66, 0x66, 0x66, 0x66, 0x66,
	0x66, 0x80, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x08,
	0x66, 0x66, 0x66, 0x66, 0x80, 0x00, 0x00, 0x00, 0x08, 0x66, 0x66, 0x80,
	0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0xC0, 0x03,
	0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01,
	0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0xC0, 0x03, 0x00, 0x00, 0xE0, 0x07,
	0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00
};


byte ip[]      = { 192,168,  0, 30 }; //ip from shield (server)
byte netmask[] = { 255,255,255,  0 }; //netmask

//initialize the server library with the port 
//you want to use (port 80 is default for HTTP)
RedFlyServer server(80);
//initialize the NBNS library with the device name (max. 16 characters)
RedFlyNBNS NBNS("REDFLY");


//debug output functions (9600 Baud, 8N2)
//Leonardo boards use USB for communication, so we dont need to disable the RedFly
void debugout(char *s)
{
#if defined(__AVR_ATmega32U4__)
  Serial.print(s);
#else
  RedFly.disable();
  Serial.print(s);
  RedFly.enable();
#endif
}

void debugoutln(char *s)
{
#if defined(__AVR_ATmega32U4__)
  Serial.println(s);
#else
  RedFly.disable();
  Serial.println(s);
  RedFly.enable();
#endif
}


void setup()
{
  uint8_t ret;

#if defined(__AVR_ATmega32U4__) //Leonardo boards use USB for communication
  Serial.begin(9600); //init serial port and set baudrate
  while(!Serial); //wait for serial port to connect (needed for Leonardo only)
#endif

  //init the WiFi module on the shield
  // ret = RedFly.init(br, pwr) //br=9600|19200|38400|57600|115200|200000|230400, pwr=LOW_POWER|MED_POWER|HIGH_POWER
  // ret = RedFly.init(pwr) //9600 baud, pwr=LOW_POWER|MED_POWER|HIGH_POWER
  // ret = RedFly.init() //9600 baud, HIGH_POWER
  ret = RedFly.init();
  if(ret)
  {
    debugoutln("INIT ERR"); //there are problems with the communication between the Arduino and the RedFly
  }
  else
  {
    //scan for wireless networks (must be run before join command)
    RedFly.scan();

    //join network
    // ret = join("wlan-ssid", "wlan-passw", INFRASTRUCTURE or IBSS_JOINER or IBSS_CREATOR, chn, authmode) //join infrastructure or ad-hoc network, or create ad-hoc network
    // ret = join("wlan-ssid", "wlan-passw", IBSS_CREATOR, chn) //create ad-hoc network with password, channel 1-14
    // ret = join("wlan-ssid", IBSS_CREATOR, chn) //create ad-hoc network, channel 1-14
    // ret = join("wlan-ssid", "wlan-passw", INFRASTRUCTURE or IBSS_JOINER) //join infrastructure or ad-hoc network with password
    // ret = join("wlan-ssid", INFRASTRUCTURE or IBSS_JOINER) //join infrastructure or ad-hoc network
    // ret = join("wlan-ssid", "wlan-passw") //join infrastructure network with password
    // ret = join("wlan-ssid") //join infrastructure network
    ret = RedFly.join("wlan-ssid", "wlan-passw", INFRASTRUCTURE);
    if(ret)
    {
      debugoutln("JOIN ERR");
      for(;;); //do nothing forevermore
    }
    else
    {
      //set ip config
      // ret = RedFly.begin(); //DHCP
      // ret = RedFly.begin(1 or 2); //1=DHCP or 2=Auto-IP
      // ret = RedFly.begin(ip);
      // ret = RedFly.begin(ip, dnsserver);
      // ret = RedFly.begin(ip, dnsserver, gateway);
      // ret = RedFly.begin(ip, dnsserver, gateway, netmask);
      ret = RedFly.begin(ip, 0, 0, netmask);
      if(ret)
      {
        debugoutln("BEGIN ERR");
        RedFly.disconnect();
        for(;;); //do nothing forevermore
      }
      else
      {
        server.begin();
      }
    }
  }
}


char* strrmvspace(char *dst, const char *src) //remove space at start and end of a string
{
  uint16_t i;

  if(*src == 0)
  {
    *dst = 0;
  }
  else
  {
    //at start
    for(i=0; isspace(src[i]); i++);
    strcpy(dst, &src[i]);
    //at end
    i=strlen(dst)-1;
    for(i=strlen(dst)-1; isspace(dst[i]); i--)
    {
      dst[i] = 0;
    }
  }

  return dst;
}


uint16_t htou(const char *src) //hex to uint
{
  uint16_t value=0;

  while(*src)
  {
         if((*src >= '0') && (*src <= '9')) { value = (value*16) + (*src-'0'+ 0); }
    else if((*src >= 'A') && (*src <= 'F')) { value = (value*16) + (*src-'A'+10); }
    else if((*src >= 'a') && (*src <= 'f')) { value = (value*16) + (*src-'a'+10); }
    else                                    { break; }
    src++;
  }

  return value;
}


uint16_t url_decode(char *dst, const char *src, uint16_t len)
{
  uint16_t i;
  char c, *ptr, buf[3]={0,0,0};

  ptr = dst; //save dst

  for(i=0; i<len;)
  {
    c = *src++; i++;
    if((c == 0)    || 
       (c == '&')  ||
       (c == ' ')  ||
       (c == '\n') ||
       (c == '\r'))
    {
      break;
    }
    else if(c == '%')
    {
      buf[0] = *src++; i++;
      buf[1] = *src++; i++;
      *dst++ = (unsigned char)htou(buf);
    }
    else if(c == '+')
    {
      *dst++ = ' ';
    }
    else
    {
      *dst++ = c;
    }
  }
  *dst = 0;

  //remove space at start and end of string
  strrmvspace(ptr, ptr);

  return i;
}


void loop()
{
  char file[32], param[64], *ptr, *p1, *p2, *p3;
  int c, i, len;

  //NBNS service routine
  NBNS.service();

  //listen for data (HTTP server)
  if(server.available() > 10)
  {
    //analyze HTTP header/request
    file[0] = 0;
    server.read((uint8_t*)file, 4);
    if(strncmp_P(file, PSTR("GET "), 4) == 0) 
    {
      //get file
      file[0] = 0;
      ptr = file;
      do
      {
        c = server.read();
        if(isalnum(c) || (c == '/'))
        {
          *ptr++ = c;
          *ptr   = 0;
        }
      }while((c != ' ') && (c != '?') && (c != -1));

      //get parameter
      param[0] = 0;
      p1       = 0;
      p2       = 0;
      p3       = 0;
      if(c == '?')
      {
        //read parameter
        ptr = param;
        len = 0;
        do
        {
          c = server.read();
          if((c != ' ') && (len < (sizeof(param)-1)))
          {
            *ptr++ = c;
            *ptr   = 0;
            len++;
          }
        }while((c != ' ') && (c != '\n') && (c != '\r') && (c != -1));

        //decode parameter
        ptr = param;
        while(len != 0)
        {
               if(strncasecmp(ptr, "p1=", 3) == 0)
          { ptr+=3; len-=3; p1=ptr; i=url_decode(ptr, ptr, len); ptr+=i; len-=i; }
          else if(strncasecmp(ptr, "p2=", 3) == 0)
          { ptr+=3; len-=3; p2=ptr; i=url_decode(ptr, ptr, len); ptr+=i; len-=i; }
          else if(strncasecmp(ptr, "p3=", 3) == 0)
          { ptr+=3; len-=3; p3=ptr; i=url_decode(ptr, ptr, len); ptr+=i; len-=i; }
          else
          {
            ptr++; 
            len--;
          }
        }
      }

      //clear input buffer
      server.flush();

      //send file
      if(((file[0] == '/') && (file[1] == 0))          ||
         (strncasecmp_P(file, PSTR("/index"), 6) == 0) ||
         (strncasecmp_P(file, PSTR("/test"),  5) == 0) ||
         (strncasecmp_P(file, PSTR("/abc"),   4) == 0)) //default file
      {
        //send standard HTTP 200 header
        server.print_P(PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));

        //send HTML
        server.println_P(PSTR("<h2>RedFly Test</h2>\r\n" \
                              "<a href=\"/index\">index</a> <a href=\"/test\">test</a> <a href=\"/abc\">abc</a> <a href=\"/favicon\">favicon</a><br><br>\r\n" \
                              "<form method=\"get\">\r\n" \
                              "P1: <input type=\"text\" name=\"p1\" maxlength=\"10\" value=\"\"><br>\r\n" \
                              "P2: <input type=\"text\" name=\"p2\" maxlength=\"10\" value=\"\"><br>\r\n" \
                              "P3: <input type=\"text\" name=\"p3\" maxlength=\"10\" value=\"\"><br>\r\n" \
                              "<input type=\"submit\"></form>\r\n" \
                              "<h3>Info</h3>"));

        if(file[0])
        {
          server.print_P(PSTR("FILE: "));
          server.print(file);
          server.println_P(PSTR("<br>"));
        }
        if(p1)
        {
          server.print_P(PSTR("P1: "));
          server.print(p1);
          server.println_P(PSTR("<br>"));
        }
        if(p2)
        {
          server.print_P(PSTR("P2: "));
          server.print(p2);
          server.println_P(PSTR("<br>"));
        }
        if(p3)
        {
          server.print_P(PSTR("P3: "));
          server.print(p3);
          server.println_P(PSTR("<br>"));
        }
      }
      else if(strncasecmp_P(file, PSTR("/favicon"), 8) == 0) //favicon
      {
        //send standard HTTP 200 header
        server.print_P(PSTR("HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\n\r\n"));
        //send image data (from Flash)
        PGM_P data;
        data = (PGM_P)FAVICON;
        for(len=FAVICONSIZE; len!=0;)
        {
          if(len >= 32)
          {
            memcpy_P(file, data, 32);
            server.write((uint8_t*)file, 32);
            data += 32;
            len  -= 32;
          }
          else
          {
            memcpy_P(file, data, len);
            server.write((uint8_t*)file, len);
            data += len;
            len   = 0;
          }
        }
      }
      else //file not found
      {
        //send HTTP 404 header
        server.print_P(PSTR("HTTP/1.1 404 Not Found\r\n\r\nError 404 Not Found"));
      }
    }
    else
    {
      //clear input buffer
      server.flush();
      //send HTTP 400 header
      server.print_P(PSTR("HTTP/1.0 400 Bad request\r\n\r\nError 400 Bad request"));
    }

    //close connection
    server.stop();
  }
  else if(!server.connected()) //listening port still open?
  {
    server.stop(); //stop and reset server
    server.begin(); //start server
  }
}
