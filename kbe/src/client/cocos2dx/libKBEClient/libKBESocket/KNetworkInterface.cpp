//
//  KNetworkInterface.cpp
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#include "KNetworkInterface.h"
#include "KBEGameSocket.h"
#include "net/message.h"
#include "KMessage.h"

namespace KBEngineClient
{
	void KNetworkInterface::Connect()
	{

	}

	void KNetworkInterface::send(uint8* data,int size)
	{

		KBEGameSocket::getInstance().sendData((char*)data,size);

		//TODO decode it and send it
		/*Message* msg = new Message();
		msg->setOpcode(5);
		KBEGameSocket::getInstance().sendMessage(*msg);*/

	}

	void KNetworkInterface::recv()
	{

	}

	void KNetworkInterface::bindMessage()
	{
		
	}

}