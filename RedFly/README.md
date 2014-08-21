# RedFly-Shield

## FAQ

### Howto update the Firmware?
See the [Firmware Update Guide](https://github.com/watterott/RedFly-Shield/blob/master/fw/update_guide/README.md).

### Which pins does the RedFly use?
Used pins: D0, D1, D2, D3

The RedFly also uses the first hardware serial port (data format 8N2) of the Arduino (D0+D1).
If you want to use this serial port for other things then you have to disable the communication with the RedFly at first (see *enable()* and *disable()*).

### Why I am getting errors when I send commands/data to the RedFly?
When sending commands to the RedFly you always have to check that the serial input buffer doesn't contain data (see *socketRead()*, *client.read()* and *server.read()*).

### The received data is not correct?
If you have receiving problems, e.g. on a high data volume, then set a higher communication baud rate with the *init()* function.

### A connection cannot be established to the RedFly?
Check if your AP has a *client isolation* option and if it is disabled.
Also some customers reported problems with AVM FritzBox routers as AP. In this case disable the automatic WiFi power managment and try with maximum WiFi power and fixed data rate.

### The WiFi connection does not work (*join()* or *begin()* return an error)?
In high power transmit mode an external power supply is recommended, because in some cases the USB port has not enough power.

### The initialization of the WiFi module does not work (*init()* returns a value >0)?
Try to update/re-flash the WiFi module firmware: [Firmware Update Guide](https://github.com/watterott/RedFly-Shield/tree/master/fw/update_guide)

### What is the wireless range of the RedFly?
The range varies depending on the environment (indoor, outdoor, other nearby WiFi networks).
The indoor range is about 10-20 meters and the outdoor range is up to 200 meters.

### What WiFi module is on the RedFly?
It is a Redpine Signals RS9110-N-11-22 with UART interface.


## Examples

Examples can be found in the Arduino IDE under ```File -> Examples -> RedFly```.

French description and additional functions:
* http://www.mon-club-elec.fr/pmwiki_reference_arduino/pmwiki.php?n=Main.LibrairieRedFly
* http://www.mon-club-elec.fr/pmwiki_mon_club_elec/pmwiki.php?n=MAIN.MATERIELArduinoShieldWifiRedflyShield

DHCP and DNS servers for RedFly:
* http://notebook.kulchenko.com/embedded/dhcp-and-dns-servers-with-arduino
* https://github.com/pkulchenko/DHCPLite


## Libraries


### RedFly

    uint8_t init(uint32_t br, uint8_t pwr)
    uint8_t init(uint8_t pwr) //br=9600
    uint8_t init(void) //br=9600, pwr=HIGH_POWER
* br - Baud rate: 9600, 19200, 38400, 57600, 115200, 200000, 230400, 460800, 921600, 1843200, 3686400 (only baud rates up to 230400 are support by Arduino @ 16MHz)
* pwr - Transmitter power and data rate:
  * LOW_POWER,     MED_POWER,     HIGH_POWER     (auto data rate)
  * LOW_POWER_1M,  MED_POWER_1M,  HIGH_POWER_1M  (1Mbps data rate)
  * LOW_POWER_2M,  MED_POWER_2M,  HIGH_POWER_2M  (2Mbps data rate)
  * LOW_POWER_11M, MED_POWER_11M, HIGH_POWER_11M (11Mbps data rate)
  * LOW_POWER_12M, MED_POWER_12M, HIGH_POWER_12M (12Mbps data rate)
  * LOW_POWER_24M, MED_POWER_24M, HIGH_POWER_24M (24Mbps data rate)
  * LOW_POWER_54M, MED_POWER_54M, HIGH_POWER_54M (54Mbps data rate)

Initialize the WiFi module and set baud rate and transmitter power. Returns 0 if everything is okay.

    void enable(void)
Enable communication with the RedFly-Shield.
Note: The function clears the serial input and output buffer.

    void disable(void)
Disable communication with the RedFly-Shield.
Note: The function clears the serial input and output buffer.

    uint8_t getversion(char *ver)
* ver - Pointer to a buffer to receive the version

Get the firmware version. The return value is 0 if the version is successfully copied to *ver*.

    uint8_t getmac(uint8_t *mac)
* mac - Pointer to a buffer to receive the address

Get the MAC address of the WiFi module. The return value is 0 if the MAC is successfully copied to *mac*.

    uint8_t getlocalip(uint8_t *ip)
* ip - Pointer to a buffer to receive the address

Get the IP address of the WiFi module. The return value is 0 if the IP is successfully copied to *ip*.

    uint8_t getip(char *host, uint8_t *ip)
* host - host name
* ip - Pointer to a buffer to receive the IP address

Returns 0 if the ip address of the searched host is succefully received. (FW >= 4.3.0 required)

    uint32_t gettime(uint8_t *server, uint16_t port)
    uint32_t gettime(uint8_t *server) //use port 123
* server - IP address of NTP server
* port - Port of NTP server

Returns the seconds since 1970 or 0 (zero) on an error.

    uint8_t getrssi(void)
Returns the signal strengh (in dBm) of the current connection.

    uint8_t getbssid(char *ssid, uint8_t *mac)
* ssid - Pointer to a buffer to receive the SSID
* mac - Pointer to a buffer to receive the MAC address

Get the SSID and MAC of the scanned network. The return value is 0 if no error occurred.
Note: Run this function direct after *scan()* or *nextscan()*.

    uint8_t gettype(char *ssid, uint8_t *type)
* ssid - Pointer to a buffer to receive the SSID
* type - Pointer to a buffer to receive the type (0=Ad-hoc, 1=Infrastructure)

Get the SSID and type of the scanned network. The return value is 0 if no error occurred.
Note: Run this function direct after *scan()* or *nextscan()*.

    uint8_t scan(uint8_t chn, char *ssid, uint8_t *mode, uint8_t *rssi)
    uint8_t scan(char *ssid, uint8_t *mode, uint8_t *rssi)
    uint8_t scan(char *ssid)
    uint8_t scan(void)
* chn - Channel (0=all channels)
* ssid - Pointer to a buffer to receive the SSID (if set on function call, the module scans for networks with hidden SSID)
* mode - Pointer to a buffer to receive the mode value 0=Open, 1=WPA, 2=WPA2, 3=WEP
* rssi - Pointer to a buffer to receive the RSSI value (signal strengh)

Scan for wireless networks and return SSID, Mode and RSSI. If the scanning was successful the return vaule is 0.
Note: A scan cannot be started after joining a network.

    uint8_t nextscan(char *ssid, uint8_t *mode, uint8_t *rssi)
    uint8_t nextscan(char *ssid)
* ssid - SSID
* mode - Mode
* rssi - RSSI

Scan for next wireless networks and return SSID, Mode and RSSI. If the scanning was successful the return vaule is 0.
Note: Run this function direct after *scan()* and a scan cannot be started after joining a network.

    uint8_t join(char *ssid, char *key, uint8_t net, uint8_t chn, uint8_t authmode) //infrastructure or IBSS joiner / creator
    uint8_t join(char *ssid, char *key, uint8_t net, uint8_t chn) //IBSS creator
    uint8_t join(char *ssid, char *key, uint8_t net) //infrastructure or IBSS joiner
    uint8_t join(char *ssid, uint8_t net, uint8_t chn) //IBSS creator
    uint8_t join(char *ssid, uint8_t net) //infrastructure or IBSS joiner
    uint8_t join(char *ssid, char *key) //infrastructure
    uint8_t join(char *ssid) //infrastructure
* ssid - SSID
* key - Key / Password (max 31 characters, WEP-64bit: 10 hex digits and WEP-128bit: 26 hex digits)
* net - Network: INFRASTRUCTURE, IBSS_JOINER, IBSS_CREATOR
* chn - Channel: 0...14, 0=all channels
* authmode - Auth mode: 0=Open Key, 1=Shared Key (only for WEP)

Join wireless network. If everything is okay then the return value is 0.
Note: *scan()* has to be called before this function for infrastructure and IBSS joiner networks. The maximum number of IBSS_JOINER devices (ad-hoc) is 4.

    uint8_t disconnect(void)
Disconnect / disassociate wireless connection.

    uint8_t begin(uint8_t dhcp, uint8_t *ip, uint8_t *dns, uint8_t *gateway, uint8_t *netmask)
    uint8_t begin(uint8_t *ip, uint8_t *dns, uint8_t *gateway, uint8_t *netmask)
    uint8_t begin(uint8_t *ip, uint8_t *dns, uint8_t *gateway) //netmask=255.255.255.0
    uint8_t begin(uint8_t *ip, uint8_t *dns) //netmask=255.255.255.0
    uint8_t begin(uint8_t *ip) //netmask=255.255.255.0
    uint8_t begin(uint8_t dhcp) //Use 1=DHCP or 2=Auto-IP to get IP config.
    uint8_t begin(void) //Use DHCP to get IP config.
* dhcp - 1=DHCP or 2=Auto-IP (only in infrastructure mode available, Auto-IP requires FW >= 4.3.0)
* ip - IP address
* dns - DNS server (FW >= 4.3.0 required)
* gateway - Gateway address
* netmask - Network mask

Set IP configuration. If everything is okay then the return value is 0.

    uint8_t socketConnect(uint8_t proto, uint8_t *ip, uint16_t port, uint16_t lport)
    uint8_t socketConnect(uint8_t proto, uint8_t *ip, uint16_t port) //lport=1024
* proto - Protocol: PROTO_TCP, PROTO_UDP, PROTO_MCAST (Multicast requires FW >= 4.3.0)
* ip - Server IP address
* port - Server port
* lport - Local port

Connect to server. Returns the socket handle or 0xFF on an error.

    uint8_t socketListen(uint8_t proto, uint16_t lport)
* proto - Protocol: PROTO_TCP, PROTO_UDP
* lport - Local port

Listen on port. Returns the socket handle or 0xFF on an error.
Note: If you want to open multiply sockets with the same port number, you have to wait till the opened socket gets connected and then you can open a new socket with the same port number.

__Example for using 2 sockets with the same port number:__
```
    uint8_t socket1=INVALID_SOCKET, socket2=INVALID_SOCKET; //socket handles
    
    void setup()
    {
      //init RedFly
      //...
      socket1 = RedFly.socketListen(PROTO_TCP, 123); //open 1st socket
    }
    
    void loop()
    {
      uint8_t s=0xFF; //return data from all sockets
      uint16_t len=0;
      RedFly.socketRead(&s, &len, 0, 0, 0, 0);
      if(len > 0) //data available?
      {
        if(s == socket1)
        {
          if(RedFly.socketClosed(socket2)) //check if 2nd socket is closed
          {
            socket2 = RedFly.socketListen(PROTO_TCP, 123); //open 2nd socket
          }
          //do something...
        }
        else if(s == socket2)
        {
          if(RedFly.socketClosed(socket1)) //check if 1st socket is closed
          {
            socket1 = RedFly.socketListen(PROTO_TCP, 123); //open 1st socket
          }
          //do something...
        }
      }
    }
```

    uint8_t socketClose(uint8_t socket)
* socket - Socket handle

Close socket.

    uint8_t socketClosed(uint8_t socket)
* socket - Socket handle

Is the socket closed? A return value >0 means that the socket is closed.

    uint8_t socketStatus(uint8_t socket)
* socket - Socket handle

Get socket status from WiFi module (0xFF=error).

    uint8_t socketState(uint8_t socket)
* socket - Socket handle

Get socket state (0x00=TCP, 0x01=UDP, 0x02=MCAST, 0xFF=closed).

    uint8_t socketSend(uint8_t socket, uint8_t *stream, uint16_t size, uint8_t *ip, uint16_t port)
    uint8_t socketSendPGM(uint8_t socket, PGM_P stream, uint8_t *ip, uint16_t port)
    uint8_t socketSend(uint8_t socket, char *stream, uint8_t *ip, uint16_t port)
    uint8_t socketSend(uint8_t socket, uint8_t *stream, uint16_t size)
    uint8_t socketSendPGM(uint8_t socket, PGM_P stream)
    uint8_t socketSend(uint8_t socket, char *stream)
    uint8_t socketSend(uint8_t socket, int value)
* socket - Socket handle
* stream - Data
* size - Data length
* value - Data (int)
* ip - Server IP address (only on UDP)
* port - Server port (only on UDP)

Send data on socket. The function returns 0 if everything is okay and 0xFF if there is new data in the input buffer (no data is sent).
The *ip* and *port* parameters are only used on UDP connections.
Note: The input buffer has to be empty before sending data (see *socketRead()*).

    uint16_t socketRead(uint8_t *socket, uint16_t *len, uint8_t *ip, uint16_t *port, uint8_t *dst, uint16_t dst_size) //UDP connection
    uint16_t socketRead(uint8_t *socket, uint16_t *len, uint8_t *dst, uint16_t dst_size)
* socket - Socket handle (set from function or if set on function call, only return data from this socket)
* len - Complete data length
* ip - Server IP address (only UDP)
* port - Server port (only UDP)
* dst - Receive buffer
* dst_size - Receive buffer length

Read data. Returns data length in dst (0xFFFF=socket closed).

__Example:__
```
    uint8_t socket=0xFF;   //0xFF = return data from all sockets, otherwise return only data from specialized socket
    uint16_t len;          //complete data length, that is available
    uint8_t ip[4];         //source IP, only on UDP connection
    uint16_t port          //source port, only on UDP connection
    uint8_t dst[64];       //destination buffer for the data
    uint16_t d_size=64;    //size of destination buffer
    uint16_t received_len; //data copied to destination buffer
    received_len = RedFly.socketRead(&socket, &len, ip, &port, dst, d_size); //ip + port only on UDP connections
    received_len = RedFly.socketRead(&socket, &len, dst, d_size);
```

#### Error codes
```
  -1 (0xFF) Waiting for the connection from peer.
  -2 (0xFE) Socket not available.
  -3 (0xFD) De-authentication from the Access Point.
  -4 (0xFC) Illegal IP/Port parameters.
  -5 (0xFB) TCP/IP configuration failure.
  -6 (0xFA) Invalid socket.
  -7 (0xF9) Association not done.
  -8 (0xF8) Error in command.
  -9 (0xF7) Error with byte stuffing for escape characters.
 -10 (0xF6) IP Lease expired.
 -11 (0xF5) TCP connection closed.
 -12 (0xF4) Pre-Shared Key contains invalid characters (applicable in case of WEP).
 -13 (0xF3) No Access Points present. This error is also issued if the security mode of the AP is different from the one supplied to the module in the "Authmode" command.
 -14 (0xF2) 1. The "Init" command is sent more than once.
            2. Flash access error for at_rsi_cfgenable command.
            3. Returned for "Cfgget" command when the config save feature is not enabled.
 -15 (0xF1) 1. The "Join" command is sent when module is already associated with an existing network.
            2. Scan command is issued after the module has already associated with a network.
            3. When there is a flash write failure for the "Cfgsave" command.
            4. Module not in connected state (for "Cfgsave" command).
 -16 (0xF0) DHCP Failure, DHCP Renewal not done by AP.
 -17 (0xEF) Baud Rate not supported.
 -18 (0xEE) Encryption mode not supported.
 -19 (0xED) Invalid channel.
 -20 (0xEC) "Snd" command is issued but there is no network in the remote terminal.
 -21 (0xEB) Authentication failure.
 -22 (0xEA) Re-join failure.
 -23 (0xE9) Country information not found in probe response from AP during scan.
 -24 (0xE8) Wrong PSK was supplied to the module.
 -25 (0xE7) Invalid network type.
 -26 (0xE6) Back ground scan cancelled.
 -28 (0xE4) Unsupported bits are set in Feature Select bitmap.
 -29 (0xE3) "Scan" command is issued before "Band" and "Init" commands.
 -30 (0xE2) Tx data rate configured in module does not match the rates supported by the target AP.
 -56 (0xC8) "Query RSSI" command issued in IBSS mode.
 -55 (0xC9) For RSSI query in unassociated state.
 -56 (0XC8) For rssi query in IBSS.
 -58 (0xC6) Gives an error if memory limit exceeds for "Load" command to load tadm2 file.
 -59 (0xC5) Unable to process command. The Host should stop sending commands to the module for some tens of millisecs.
 -69 (0xBB) The reply did not contain an answer with an IP address.
 -70 (0xBA) DNS class error.
 -72 (0xB8) The number of queries is more than 1 or the number of replies is zero.
 -73 (0xB7) The reply contained an error code.
 -74 (0xB6) The reply was truncated or not a response to a standard query.
 -75 (0xB5) The ID number of the reply did not match the ID number sent.
 -85 (0xAB) No buffer was passed or the reply was too short.
 -91 (0xA5) Invalid IP address passed or unable to send IGMP report.
 -92 (0xA4) DNS response timed out.
 -95 (0xA1) Unable to send data as ARP is not resolved.
-100 (0x9C) DHCP handshake failure.
-113 (0x8F) Unable to send data because of connectivity disruption (such as AP switched off abruptly).
-121 (0x87) Error issued when trying to connect to a non-existent TCP server socket.
-124 (0x84) Connection establishment failure.
-127 (0x81) Socket already exists.
-128 (0x80) Attempt to open more than 8 sockets.
-131 (0x7D) "DHCP Mode" parameter in "Set IP parameters" command is set to a value other than 0 or 1.
-211 (0x2D) This code is returned in the following scenario: An LTCP socket is opened in the module. The remote peer tries to connect to the socket using a client socket, by sending a SYN packet (according to TCP protocol). The module in turn responds with a SYN+ACK packet. The remote peer is now supposed to send an ACK packet. If this packet is missed or does not reach the module, the module retries sending the SYN+ACK packet 4 times, and if the maximum number of retries does not lead to reception of an ACK from the remote terminal, the module returns this error to the Host and closes the LTCP socket.
   1 (0x01) Wireless configuration failure.
  25 (0x19) Invalid value supplied as input in "Authmode" command.
  51 (0x33) Keep alive timeout. On lack of activity over an existing TCP socket connection, the module sends out a Keep alive frame to the remote terminal. If there is no response, the module closes its TCP socket and sends out an asynchronous message "ERROR<Error_code=0x33>".
  64 (0x40) Tx buffers are not available. The Host should stop sending data to the module for some tens of millisecs to relax the buffers
  65 (0x41) A packet of size more than 1460 bytes (TCP) or more than 1472 bytes (UDP) is sent in "Snd" command.
  66 (0x42) Zero TCP/UDP payload is sent from the module.
```


### RedFlyClient


    RedFlyClient()
    RedFlyClient(uint8_t socket)
    RedFlyClient(uint8_t *ip, uint16_t port)
    RedFlyClient(uint8_t *ip, uint16_t port, uint16_t lport)
* socket - Socket handle
* ip - Client ip to connect
* port - Client port to connect
* lport - Local port

Create client.

    void begin(void)
    void beginUDP(void)
    int connect(void)
    int connectUDP(void)
    int connect(uint8_t *ip, uint16_t port)
    int connectUDP(uint8_t *ip, uint16_t port)
    int connect(uint8_t *ip, uint16_t port, uint16_t lport)
    int connectUDP(uint8_t *ip, uint16_t port, uint16_t lport)
    int connect(char *host, uint16_t port)
    int connectUDP(char *host, uint16_t port)
* ip - Client ip to connect
* port - Client port to connect
* lport - Local port
* host - Host name

Start the TCP or UDP client. *connect()* returns 1 if everything is okay.

    uint8_t connected(void)
Is the Client connected? The return value is 1 if yes.

    void stop(void)
Stop the Client.

    uint8_t status(void)
Returns the Client status. 0 means an open connection.
Note: The status will be directly read from the WiFi module.

    uint8_t getsocket(void)
Returns the socket handle for the connection.

    int available(void)
Returns the size/length of available data.

    int read(void)
Get the received data. On -1 an error occurred.

    int read(uint8_t *s, size_t sz)
* s - Destination buffer
* sz - Buffer size

Returns the received data len.

    void flush(void)
Flushes the receive buffer.

    size_t write(uint8_t b)
    size_t write(const char *s)
    size_t write(const uint8_t *s, size_t sz)
* b - Byte to write
* s - String/Array to write
* sz - Array size

Write data to the connected device and return the number of written bytes.
Note: The input buffer has to be empty before sending data (see *read()*).

    size_t print(text)
    size_t print(int, base)
    size_t println(text)
    size_t println(int, base)
* text, int - String, Char, Integer, Long, Float (RAM)
* base - DEC, HEX, OCT, BIN

Print text/data from RAM.

    size_t print_P(PGM_P s)
    size_t println_P(PGM_P s)
* s - Pointer to String/Array (Flash), example: PSTR("test"), further infos about using flash memory: http://www.nongnu.org/avr-libc/user-manual/pgmspace.html

Print text/data from Flash.


### RedFlyServer

    RedFlyServer()
    RedFlyServer(uint16_t port)
* port - Server port for listening

Create server.
      
    void begin(void)
    void beginUDP(void)
    int connect(void)
    int connectUDP(void)
    int connect(uint16_t port)
    int connectUDP(uint16_t port)
* port - Server port to start listening

Start TCP or UDP server. *connect()* returns 1 if everything is okay.

    uint8_t connected(void)
Is the Server port open? The return value is 1 if yes.

    void stop(void)
Stop the Server.

    uint8_t status(void)
Returns the Server status. 0 means an open connection.
Note: The status will be directly read from the WiFi module.

    uint8_t getsocket(void)
Returns the socket handle for the connection.

    void getip(uint8_t *ip)
* ip - IP address

Returns the IP of the client on an UDP connection.
      
    uint16_t getport(void)
Returns the port of the client on an UDP connection..

    int available(void)
Returns the size/length of available data.

    int read(void)
Get the received data. On -1 an error occurred.

    int read(uint8_t *s, size_t sz)
* s - Destination buffer
* sz - Buffer size

Returns the received data len.

    void flush(void)
Flushes the receive buffer.

    size_t write(uint8_t b)
    size_t write(const char *s)
    size_t write(const uint8_t *s, size_t sz)
* b - Byte to write
* s - String/Array to write
* sz - Array size

Write data to the connected device and return the number of written bytes.
Note: The input buffer has to be empty before sending data (see *read()*).

    size_t print(text)
    size_t print(int, base)
    size_t println(text)
    size_t println(int, base)
* text, int - String, Char, Integer, Long, Float (RAM)
* base - DEC, HEX, OCT, BIN

Print text/data from RAM.

    size_t print_P(PGM_P s)
    size_t println_P(PGM_P s)
* s - Pointer to String/Array (Flash), example: PSTR("test"), further infos about using flash memory: http://www.nongnu.org/avr-libc/user-manual/pgmspace.html

Print text/data from Flash.


### RedFlyNBNS

    RedFlyNBNS()
    RedFlyNBNS(char *name)
* name - Device name
      
Create NBNS (NetBIOS Name Service).

    void setName(char *name)
    void setNamePGM(PGM_P name)
* name - Device name (String/Array, Pointer to String/Array in Flash)

Set device name.

    uint8_t service(void)
Service routine, run in *loop()*.
