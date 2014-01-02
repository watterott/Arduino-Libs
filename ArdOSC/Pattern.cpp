/*
 
 ArdOSC 2.1 - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2011 recotana( http://recotana.com )　All right reserved
 
 */

#include <stdlib.h>
#include <string.h>
#include "OSCcommon/OSCServer.h"
#include "OSCcommon/OSCcommon.h"
#include "OSCCommon/OSCMessage.h"

#include "OSCCommon/Pattern.h"



Pattern::Pattern(){
    patternNum=0;

}
Pattern::~Pattern(){    
}

void Pattern::addOscAddress(char *_adr ,  AdrFunc _func){
    adrFunc[patternNum] = _func;
    addr[patternNum] = _adr;
    patternNum++;
}

void Pattern::execFunc(uint8_t _index,OSCMessage *_mes){
    adrFunc[_index](_mes);
}

void Pattern::paternComp(OSCMessage *_mes){
    
    for (uint8_t i=0 ; i<patternNum ; i++) {
        if ( strcmp( addr[i] , _mes->_oscAddress ) == 0 ) execFunc( i , _mes );

    }
}