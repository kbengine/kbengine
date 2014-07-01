#pragma once
#include "KBundle.h"
#include "KMessage.h"
#include "KNetworkInterface.h"

namespace KBEngineClient
{

	enum MAILBOX_TYPE
	{
			MAILBOX_TYPE_CELL = 0,
			MAILBOX_TYPE_BASE = 1
	};

	class MailBox
	{
		
	public:
		int32 id;
		std::string classtype;
		MAILBOX_TYPE type;		
		KNetworkInterface networkInterface_;		
		KBundle* bundle;		
		MailBox(void):id(0),classtype(""),bundle(0),type( MAILBOX_TYPE_CELL)
		{
			//networkInterface_ = KBEngineApp.app.networkInterface();
		}
		
		virtual void __init__()
		{
		}
		
		bool isBase()
		{
			return type == MAILBOX_TYPE_BASE;
		}
	
		bool isCell()
		{
			return type == MAILBOX_TYPE_CELL;
		}
		
		KBundle* newMail()
		{  
			if(bundle == 0)
				bundle = new KBundle();
			
			if(type == MAILBOX_TYPE_CELL)
				bundle->newmessage(*KMessage::messages["Baseapp_onRemoteCallCellMethodFromClient"]);
			else
				bundle->newmessage(*KMessage::messages["Base_onRemoteMethodCall"]);
	
			bundle->writeInt32(this->id);
			
			return bundle;
		}
		
		void postMail(KBundle* inbundle)
		{
			if(inbundle == 0 )
				inbundle = bundle;
			
			inbundle->send(networkInterface_);
			
			if(inbundle == bundle)
				bundle = 0;
		}
	};//end class define
};

