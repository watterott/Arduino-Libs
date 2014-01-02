/*
 
 ArdOSC 2.1 - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2011 recotana( http://recotana.com )　All right reserved
 
 */

#include <stdlib.h>
#include <string.h>

#include "OSCcommon/OSCcommon.h"
#include "OSCcommon/OSCDecoder.h"
#include "OSCcommon/OSCArg.h"


int16_t OSCDecoder::decode( OSCMessage::OSCMessage *_newMes , const uint8_t *_binData ){
	
	
	const uint8_t *packStartPtr = _binData;
	
	_newMes->beginMessage( (char*)packStartPtr );			
	packStartPtr += _newMes->_oscAdrAlignmentSize;
	
    
    

    char *tmpTag = (char*)(packStartPtr+1);
    uint8_t argsNum = strlen(tmpTag);
    uint16_t typeTagAlignSize = CULC_ALIGNMENT(argsNum+1);
    
    packStartPtr += typeTagAlignSize;
    
    
    for (uint8_t i=0 ; i< argsNum ; i++) {
        

        switch ( tmpTag[i] ) {
                
            case kTagInt32:
                packStartPtr += _newMes->setArgData( kTagInt32 , (void*)packStartPtr , 4 , false );
            break;
                
            case kTagFloat:
                packStartPtr += _newMes->setArgData( kTagFloat , (void*)packStartPtr , 4 , false );
                break;
                
            case kTagString:
                packStartPtr += _newMes->setArgData( kTagString , (void*)packStartPtr , strlen((char*)packStartPtr) , true );
                break;
            
            default:
                break;
        }
    }
   
	return 1;
    
}

