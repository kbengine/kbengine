//
//  KMessage.h
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#ifndef __libKBEClient__KMessage__
#define __libKBEClient__KMessage__

#include "KBEClientcore.h"

namespace KBEngineClient{
	class KMessage{
	
	public:
		KMessage(uint16 msgid,char* msgname,int16 length, uint8 argstype, uint8 msgargtypes[],void*);
		KMessage(const KMessage& other);
		KMessage():id(-1),msglen(0),msgname()
		{
			printf(" empty message created.!");
		};
		static std::map<std::string,KMessage*> messages;
		static std::map<std::string,KMessage*> loginappMessages;
		static std::map<std::string,KMessage*> baseappMessages;
		static std::map<uint16,KMessage*> clientMessages;
		static void bindmessage();
		uint16 id;
		std::string msgname;
		uint16 msglen;
		uint8 argstype;
		uint8* msgargtypes;

		//KMessage& operator=(const KMessage& other)
		//{ 
		//	this->id = other.id; this->msglen = other.msglen;
		//	this->msgname = other.msgname; 
		//	this->argstype = other.argstype;
		//	return *this;
		//};
	}; 

}

#endif /* defined(__libKBEClient__KMessage__) */
