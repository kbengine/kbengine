//
//  KNetworkInterface.cpp
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#include "KBEApplication.h"
#include "KMessage.h"

namespace KBEngineClient
{

void KBEngineClient::KBE_init()
{
	//start socket. start recvThread. or sendThread.
	//connect to game server. works like client app. (handle state auto login_app, auto import )
	//while true
	//  select(socket_,&reads,&writes)
	//	if recvThread.
	//	if sendThread. always sent .. (from Bundle's buff)  
	//  HandleMessage(MemoryStream &s)
	//  msgid == ""
	//    >> Handle_onEntityEnterWorld(i,stream)  > clientApp.onEnterWorld()
	//    >> 
	// 
	 
	KMessage::bindmessage();

	//Test send ["Loginapp_importClientMessages"] 


}

void KBEngineClient::KBE_run()
{

	//Packet* packet = MessageReader.tryGetPacket(stream);
	////done.
	//if(packet->msgID_ == 1)
	//	CoreEvent::dispatch("");
	////done

}

}