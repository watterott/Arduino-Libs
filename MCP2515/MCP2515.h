/*
  MCP2515.h - CAN library
  Written by Frank Kienast in November, 2010
  
  Connections to MCP2515:
  Arduino  MCP2515
  11       MOSI
  12       MISO
  13       SCK
  10       CS 

*/

#ifndef MCP2515_h
#define MCP2515_h

#if ARDUINO >= 100
# include "Arduino.h"
#else
# include "WProgram.h"
#endif


typedef struct
{
  boolean isExtendedAdrs;
  unsigned long adrsValue;
  boolean rtr;
  byte dataLength;
  byte data[8];
} CANMSG;


class MCP2515
{
  public:
    static boolean initCAN(int baudConst);
    static boolean setCANNormalMode(boolean singleShot);
    static boolean setCANReceiveonlyMode();
    static boolean receiveCANMessage(CANMSG *msg, unsigned long timeout);
    static boolean transmitCANMessage(CANMSG msg, unsigned long timeout);
    static byte getCANTxErrCnt();
    static byte getCANRxErrCnt();
    static long queryOBD(byte code);
	
	private:
    static boolean setCANBaud(int baudConst);
    static void writeReg(byte regno, byte val);
    static void writeRegBit(byte regno, byte bitno, byte val);
    static byte readReg(byte regno);
};


//Data rate selection constants
#define CAN_BAUD_10K 1
#define CAN_BAUD_50K 2
#define CAN_BAUD_100K 3
#define CAN_BAUD_125K 4
#define CAN_BAUD_250K 5
#define CAN_BAUD_500K 6


#endif //MCP2515_h
