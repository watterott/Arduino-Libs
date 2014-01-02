/*
 
 ArdOSC 2.1 - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2011 recotana( http://recotana.com )　All right reserved
 
 */				   

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "OSCcommon/OSCcommon.h"
#include "OSCCommon/OSCMessage.h"


OSCMessage::OSCMessage(void)
{	
	_oscAddress = NULL;
    _oscAdrSize = 0;
	_oscAdrAlignmentSize = 0;
    _argsNum = 0;
	_argsAlignmentSize = 0;	
	_port = 0;
}

OSCMessage::OSCMessage(const char *_oscAddr)
{
	_argsNum = 0;
	_argsAlignmentSize = 0;	
	_port = 0;
	
	setOSCAddress(_oscAddr);
}

OSCMessage::~OSCMessage(void)
{
	flush();
}


void OSCMessage::flush(void){
	
    if ( _oscAdrSize == 0 ) return;
    
    free(_oscAddress);
    _oscAddress = NULL;
    _oscAdrSize = 0;
	_oscAdrAlignmentSize = 0;

    OSCArg *tmp;

    for ( uint8_t i = 0 ; i<_argsNum ; i++ ){
        tmp = _args[i];
        delete tmp;
    }
    
	_argsNum = 0;
	_argsAlignmentSize = 0;	
	_port = 0;

}

void OSCMessage::setAddress(uint8_t *_ipAddr , uint16_t _portNum){	
	setIpAddress(_ipAddr);
	_port = _portNum;
}


void OSCMessage::setIpAddress(uint8_t *_ipAddr){
    memcpy( _ip , _ipAddr , 4 ); 
}

uint8_t* OSCMessage::getIpAddress(void){
    return _ip;  
}


void OSCMessage::setPortNumber(uint16_t _portNum){
    _port = _portNum; 
}

uint16_t OSCMessage::getPortNumber(){
    return _port; 
}



int16_t OSCMessage::setOSCAddress(const char *_address){

	_oscAdrSize = (uint16_t)strlen(_address);
	
	if( _oscAdrSize > kMaxOSCAdrCharactor ){
		flush();
		return -1;
	}
	
	if( _oscAddress != NULL ) free(_oscAddress);
	
	_oscAddress = (char*)calloc(1,_oscAdrSize+1);
	strcpy( _oscAddress , _address) ;
	
	_oscAdrAlignmentSize = CULC_ALIGNMENT(_oscAdrSize);
    
	return 1;
    
}


uint16_t OSCMessage::getMessageSize(void){
    return  _oscAdrAlignmentSize + _typeTagAlignmentSize + _argsAlignmentSize; 
}

uint16_t OSCMessage::getArgAlignmentSize(uint8_t _index){
    return _args[_index]->_alignmentSize;
}

char   *OSCMessage::getOSCAddress(void){
    return _oscAddress; 
}

int16_t OSCMessage::getArgsNum(void){
    return _argsNum; 
}

char    OSCMessage::getArgTypeTag(int16_t _index)   {
    
    if(_index>_argsNum) return -1;
    
    return _args[_index]->_typeTag; 
    
}


void OSCMessage::swap(uint8_t *data1, uint8_t *data2){
    data1[0] = data2[3];
    data1[1] = data2[2];
    data1[2] = data2[1];
    data1[3] = data2[0];
}






int16_t OSCMessage::beginMessage(const char *_address){
    flush();
    return setOSCAddress(_address);
}

int16_t OSCMessage::setArgData(char _type , void *_value , uint8_t _byte,bool _enableAlignment){
    
    _args[_argsNum]= new OSCArg( _type , _value , _byte , _enableAlignment );
    
    uint16_t alignSize = _args[_argsNum]->_alignmentSize;
    
    _argsAlignmentSize += alignSize;
    
    _argsNum++;
    
    _typeTagAlignmentSize = CULC_ALIGNMENT( _argsNum + 1 );
    
    return alignSize;
}




int16_t OSCMessage::addArgInt32(int32_t _value){
    
    if( _argsNum > kMaxAugument ) return -1;
    
    uint8_t tmpValue[4];
    uint8_t *data = (uint8_t*)&_value;
    swap( tmpValue , data );

    setArgData( kTagInt32 , tmpValue , 4 , false );
    
    return 1;

}

int32_t OSCMessage::getArgInt32(int16_t _index){
    
    if ( _index > _argsNum ) return -1;
    
     
    uint8_t tmpValue[4];
    uint8_t *_bin = (uint8_t*)_args[_index]->_argData;
    swap( tmpValue , _bin );
    
    return *(int32_t*)tmpValue;
}



#ifdef _USE_FLOAT_

int16_t OSCMessage::addArgFloat(float _value){
    
    if( _argsNum > kMaxAugument ) return -1;
    
    uint8_t tmpValue[4];
    uint8_t *data = (uint8_t*)&_value;
    swap( tmpValue , data );
    
    setArgData( kTagFloat , tmpValue , 4 , false );
     
    return 1;
}

float OSCMessage::getArgFloat(int16_t _index){
    
    if ( _index > _argsNum ) return -1;
    
     
    uint8_t tmpValue[4];
    uint8_t *_bin = (uint8_t*)_args[_index]->_argData;
    swap( tmpValue , _bin ); 
    
    return *(float*)tmpValue;
    
}
#endif



#ifdef _USE_STRING_
int16_t OSCMessage::addArgString(const char* _value){
    
    if (_argsNum > kMaxAugument ) return -1;
    
    setArgData( kTagString , (void*)_value , strlen(_value) , true );
        
    return 1;
}

int16_t  OSCMessage::getArgString(int16_t _index , char *_rcvstr){
    
    if ( _index > _argsNum ) return -1;
    
    strcpy( _rcvstr , (char*)_args[_index]->_argData );
    
    return 1;

}
int16_t  OSCMessage::getArgStringSize(int16_t _index){
    
    if ( _index > _argsNum ) return -1;
    
    return _args[_index]->_dataSize+1;
}
#endif

