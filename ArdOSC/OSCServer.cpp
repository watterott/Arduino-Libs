/*
 
 ArdOSC 2.1 - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2011 recotana( http://recotana.com )　All right reserved
 
 */

#include <stdlib.h>

#include "OSCcommon/OSCServer.h"


OSCServer::OSCServer(void) : udpserver()
{

}


OSCServer::~OSCServer(void)
{
    stop();
}


int OSCServer::begin(uint16_t _receivePort)
{
    if( udpserver.connectUDP(_receivePort) != 1 )
    {
      return -1;
    }

    return 1;
}


void OSCServer::stop(void){

    udpserver.stop();
}


int OSCServer::aviableCheck(void)
{
    if( udpserver.available() == 0 )
    {
      return -1;
    }

    OSCMessage rcvMes;

    //get max receive data
    udpserver.read(_rcvData, kMaxRecieveData); 
    //get client ip
    udpserver.getip(rcvMes._ip);
    //get client port
    rcvMes.setPortNumber(udpserver.getport());
    //clear input buffer
    udpserver.flush(); 

    //decode message
    if( _decoder.decode(&rcvMes, _rcvData) < 0 )
    {
      return -1;
    }

    _adrMatch.paternComp(&rcvMes);

    return 1;
}


void OSCServer::addCallback(char *_adr , Pattern::AdrFunc _func )
{
    _adrMatch.addOscAddress(_adr, _func);
}
