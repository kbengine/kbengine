//
//  KMessage.cpp
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#include "KMessage.h"
#include "cocos2d.h"
#include "..\..\..\cocos2dx\include\ccMacros.h"

// create static message template.  


std::map<std::string,KMessage*> KBEngineClient::KMessage::messages;
std::map<std::string,KMessage*> KBEngineClient::KMessage::loginappMessages;
std::map<std::string,KMessage*> KBEngineClient::KMessage::baseappMessages;
std::map<uint16,KMessage*> KBEngineClient::KMessage::clientMessages;


void KBEngineClient::KMessage::bindmessage()
{
	if(KMessage::messages.size() == 0)
		{
			//KBEngineClient::KMessage::messages["Loginapp_importClientMessages"] = new KMessage(5, "importClientMessages", 0, 0, 0, NULL) ;

			KMessage::messages["Loginapp_importClientMessages"] = new KMessage(5, "importClientMessages", 0, 0, NULL, NULL);
			CCAssert( KMessage::messages["Loginapp_importClientMessages"]->id == 5 ,"");
			KMessage::messages["Baseapp_importClientMessages"] = new KMessage(207, "importClientMessages", 0, 0, NULL, NULL);
			KMessage::messages["Baseapp_importClientEntityDef"] = new KMessage(208, "importClientMessages", 0, 0,NULL, NULL);
				
			KMessage::messages["Client_onImportClientMessages"] = new KMessage(518, "Client_onImportClientMessages", -1, -1, NULL,NULL); 
			//	this.app_.GetType().GetMethod("Client_onImportClientMessages"));
			KMessage::clientMessages[KMessage::messages["Client_onImportClientMessages"]->id] = KMessage::messages["Client_onImportClientMessages"];
		}
}


KBEngineClient::KMessage::KMessage(const KMessage& other)
{
	this->id = other.id;
	this->msglen = other.msglen;
	this->msgname = other.msgname;
	this->argstype = other.argstype;
	//this->msgargtypes = uint8[this->msglen];
}

KBEngineClient::KMessage::KMessage( uint16 msgid,char* msgname,int16 length, uint8 argstype, uint8* msgargtypes,void* )
{
	this->id = msgid;
	this->msglen = length;
	this->msgname = msgname;
	this->argstype = argstype;
	this->msgargtypes = msgargtypes;
}
