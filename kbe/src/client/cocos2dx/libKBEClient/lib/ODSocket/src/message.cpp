/*
 *  message.cpp
 *  test
 *
 *  Created by admin on 11-8-18.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "message.h"
#include "SocketClient.h"





Message::Message():data(NULL){
    
}
Message::~Message()
{
	//printf("Message::~Message() %p\n",this);
	if (data!=NULL) {
		delete[] data;
	}
}

int Message::datalength()
{
    return SocketClient::bytesToInt(length)+13;
//    int addr = length[3] & 0xFF;
//    
//    addr |= ((length[2] << 8) & 0xFF00);
//    
//    addr |= ((length[1] << 16) & 0xFF0000);
//    
//    addr |= ((length[0] << 24) & 0xFF000000);
//    
//    return addr+17;
}
