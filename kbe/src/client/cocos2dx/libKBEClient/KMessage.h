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
#include "KMemoryStream.h"

namespace KBEngineClient{

	typedef uint16 MessageID;
	typedef uint8  Byte ;

	class KMessage{
	
	public:
		KMessage(uint16 msgid,char* msgname,int16 length, uint8 argstype, uint8 msgargtypes[],void*);
		KMessage(const KMessage& other);
		KMessage():id(0),msglen(-1),msgname(),argstype(0)
		{
			printf(" empty message created.!");
		};
		static std::map<std::string,KMessage*> messages;
		static std::map<uint16,KMessage*> loginappMessages;
		static std::map<uint16,KMessage*> baseappMessages;
		static std::map<uint16,KMessage*> clientMessages;
		static void bindmessage();
		static void onImportClientMessages( MemoryStream& stream );
		static void onImportClientMessagesCompleted();
		static void Client_onImportMercuryErrorsDescr(MemoryStream& stream);
		static void Client_onVersionNotMatch(MemoryStream& stream);
		static void Client_onHelloCB(MemoryStream& stream);
		static void Client_onLoginSuccessfully(MemoryStream& stream);
		static void login_baseapp( bool noconnect );
		static void onImportEntityDefCompleted();
		static void Client_onCreatedProxies(MemoryStream& stream);
		static void Client_onLoginGatewayFailed(uint16 failedcode);
		static void onImportClientEntityDef( MemoryStream& stream );
		static void createDataTypeFromStream( MemoryStream& stream, bool canprint );
		static void sendActiveAck();
		static void Client_onRemoteMethodCall( MemoryStream& s );
		uint16 id;
		std::string msgname;
		int16 msglen; //?
		uint8 argstype;
		uint8* msgargtypes;

		//KMessage& operator=(const KMessage& other)
		//{ 
		//	this->id = other.id; this->msglen = other.msglen;
		//	this->msgname = other.msgname; 
		//	this->argstype = other.argstype;
		//	return *this;
		//};
		
		static std::string base_ip;
		static uint16 base_port;
	}; 

}

#endif /* defined(__libKBEClient__KMessage__) */
