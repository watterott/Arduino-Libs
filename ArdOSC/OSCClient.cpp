/*
 
 ArdOSC 2.1 - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2011 recotana( http://recotana.com )　All right reserved
 
 */

#include <stdlib.h>

#include "OSCCommon/OSCClient.h"


OSCClient::OSCClient(void) : udpclient()
{

}


OSCClient::~OSCClient(void)
{

}


int OSCClient::send(OSCMessage *_message)
{
    int result = 1;
    uint16_t msgSize;
    uint8_t *sendData;

    msgSize  = _message->getMessageSize();
    sendData = (uint8_t*) calloc(msgSize, 1);

    if( sendData == NULL )
    {
      return -1;
    }

    if( encoder.encode(_message, sendData) < 0 )
    {
      free(sendData);
      return -2;
    }

    if( udpclient.connectUDP(_message->getIpAddress(), _message->getPortNumber(), kDummyPortNumber) != 1 )
    {
      result = -3;
    }

    if( udpclient.write(sendData, msgSize) == 0 )
    {
      result = -4;
    }

    udpclient.stop();

    free(sendData);

    return result;
}
