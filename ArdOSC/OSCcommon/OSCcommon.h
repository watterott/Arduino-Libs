/*
 
 ArdOSC 2.1 - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2011 recotana( http://recotana.com )　All right reserved
 
 */



#ifndef OSCcommon_h
#define OSCcommon_h

extern "C" {
#include <inttypes.h>
}

#define kMaxAugument	16
#define kMaxRecieveData	100
#define kMaxOSCAdrCharactor	255

#define CULC_ALIGNMENT(x) (x+4)&0xfffc


//======== user define ==============

#define _USE_FLOAT_

#define _USE_STRING_


//======== user define  end  ========




#endif
