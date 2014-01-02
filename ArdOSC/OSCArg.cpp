/*
 
 ArdOSC 2.1 - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2011 recotana( http://recotana.com )　All right reserved
 
 */

#include "OSCcommon/OSCArg.h"
#include "OSCCommon/OSCMessage.h"
#include <stdlib.h>
#include <string.h>

OSCArg::OSCArg(void){
    _typeTag = 0;
    _argData = 0;
    _dataSize = 0;
    _alignmentSize = 0;
}

OSCArg::OSCArg(char _tag){
    _typeTag = _tag;
    _argData = 0;
    _dataSize = 0;
    _alignmentSize = 0;
}


OSCArg::OSCArg( char _tag , void *_data , uint16_t _size , bool _packSizeCulc ) {
    
    _typeTag = _tag;
    
    if( _typeTag == kTagBlob )  _typeTag = _size+4;
    else    _dataSize = _size;
    
    if( _packSizeCulc )   _alignmentSize = CULC_ALIGNMENT(_size);
    else                _alignmentSize = _size;
    
    if( _size == 0 ) return;
    
    _argData = calloc( 1, _alignmentSize );

    memcpy( (uint8_t*)_argData , _data , _size );

    
}

OSCArg::~OSCArg(void){
    
    if( _typeTag != 0 ) flush();

}

void OSCArg::flush(void){
    
    if( _dataSize > 0 ) free(_argData);

    _dataSize = 0;
    _alignmentSize = 0;
    _typeTag = 0;
}



