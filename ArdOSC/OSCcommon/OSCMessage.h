/*
 
 ArdOSC 2.1 - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2011 recotana( http://recotana.com )　All right reserved
 
 */




#ifndef OSCMessage_h
#define OSCMessage_h


#include "OSCcommon.h"
#include "OSCArg.h"

#define kTagInt32   'i'
#define kTagFloat   'f'
#define kTagString  's'
#define kTagBlob    'b'

#define kTagMidi    'm'
#define kTagRgba    'r'

#define kTagTrue    'T'
#define kTagFalse   'F'






class OSCMessage{
	
private:
    
	uint8_t		_ip[4];	
	uint16_t	_port;
	
	char	   *_oscAddress;	
	uint16_t	_oscAdrSize;
	uint16_t	_oscAdrAlignmentSize;
    
    
	uint16_t	_typeTagAlignmentSize;
    uint16_t	_argsAlignmentSize;
	
	uint16_t	_argsNum;

    OSCArg *    _args[kMaxAugument];

 
    uint16_t getMessageSize(void);
    uint16_t getArgAlignmentSize(uint8_t _index);
  
    int16_t setArgData( char _type , void *_value , uint8_t _byte,  bool _enableAlignment );
    void swap(uint8_t *data1, uint8_t *data2);
//    void get4ByteData(int16_t _index , uint8_t *_data);

public:

	OSCMessage(void);
	OSCMessage(const char *_oscAddr);
    
	~OSCMessage(void);
    

	void flush(void);
	
	void setAddress(uint8_t *_ip , uint16_t _port);
	
	void setIpAddress( uint8_t *_ip );		
    uint8_t* getIpAddress(void);
    
	void     setPortNumber(uint16_t _port);
	uint16_t getPortNumber(void);
	
	int16_t setOSCAddress(const char *_addr);
	char *  getOSCAddress(void);


    int16_t beginMessage( const char *_addr);

    int16_t getArgsNum(void);
    char    getArgTypeTag(int16_t _index);
    
	int16_t addArgInt32(int32_t _value);
    int32_t getArgInt32(int16_t _index);

#ifdef _USE_FLOAT_
    int16_t addArgFloat(float _value);
    float   getArgFloat(int16_t _index);
#endif

#ifdef _USE_STRING_
    int16_t addArgString(const char* _value);
    int16_t getArgString(int16_t _index, char *_rcvstr);
    int16_t getArgStringSize(int16_t _index);

#endif

    
	
	friend class OSCServer;
	friend class OSCClient;
	friend class OSCDecoder;
	friend class OSCEncoder;
    friend class Pattern;

	
};


#endif
