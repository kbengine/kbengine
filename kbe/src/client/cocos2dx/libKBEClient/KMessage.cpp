//
//  KMessage.cpp
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#include "KMessage.h"
#include "KBundle.h"
#include "KBEGameSocket.h"
#include "KEntitydef.h"
#include "KDataTypes.h"
#include "KEntity.h"
#include "client/KAccount.h"
#include "client/KAvatar.h"
#include "KMailbox.h"
#include "KBEApplication.h"

#include "cocos2d.h"

using namespace cocos2d;

// create static message template.  


std::map<std::string,KMessage*> KBEngineClient::KMessage::messages;
std::map<uint16,KMessage*> KBEngineClient::KMessage::loginappMessages;
std::map<uint16,KMessage*> KBEngineClient::KMessage::baseappMessages;
std::map<uint16,KMessage*> KBEngineClient::KMessage::clientMessages;

std::string KBEngineClient::KMessage::base_ip;
uint16 KBEngineClient::KMessage::base_port;

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



void KBEngineClient::KMessage::onImportClientMessages( MemoryStream& stream )
{
	
	uint16 msgcount;
	stream>>msgcount;

	printf ("KBEngine::Client_onImportClientMessages: start({%d})...", msgcount);
			
	while(msgcount > 0)
	{
		msgcount--;
				
		MessageID msgid;
		stream>>msgid;
		int16 msglen;
		stream>>msglen;
				
		std::string msgname;
		stream>>msgname;

		int8 argstype;
		stream>>argstype;
		Byte argsize;
		stream>>argsize;
		//List<Byte> argstypes = new List<Byte>();
		Byte* argstypes = new Byte[argsize];		
		for(Byte i=0; i<argsize; i++)
		{
			//argstypes.Add(stream.readUint8());
			uint8 _tmp;
			stream>>_tmp;
			argstypes[i] = _tmp;
		}
		//if no args donot call read operation!!!! otherwise "Cl" will be eated up!
		//stream>>(char*)argstypes;
				
		//System.Reflection.MethodInfo handler = null;
		bool isClientMethod = msgname.find("Client_") == 0;
				
		if(isClientMethod)
		{
		/*	handler = typeof(KBEngineApp).GetMethod(msgname);
			if(handler == null)
			{
				Dbg.WARNING_MSG(string.Format("KBEngine::onImportClientMessages[{0}]: interface({1}/{2}/{3}) no implement!", 
					currserver_, msgname, msgid, msglen));
				handler = null;
			}
			else
			{
				Dbg.DEBUG_MSG(string.Format("KBEngine::onImportClientMessages: imported({0}/{1}/{2}) successfully!", 
					msgname, msgid, msglen));
			}*/
		}
				
		if(msgname.length() > 0)
		{
			int handler = 0;
			//Todo: handle should be attach function name? or event only.

			KMessage::messages[msgname] = new KMessage(msgid, (char*) msgname.c_str() , msglen, argstype, argstypes, 0);
					
			//if(!isClientMethod)
			//	Dbg.DEBUG_MSG(string.Format("KBEngine::onImportClientMessages[{0}]: imported({1}/{2}/{3}) successfully!", 
			//		currserver_, msgname, msgid, msglen));
					
			if(isClientMethod)
			{
				KMessage::clientMessages[msgid] = KMessage::messages[msgname];
				CCLog(  "KBEngine::onImportClientMessages store {%s} ",msgname.c_str() );
			}
			else
			{
				//if(currserver_ == "loginapp")
				if(base_ip == "")
					KMessage::loginappMessages[msgid] = KMessage::messages[msgname];
				else
					KMessage::baseappMessages[msgid] = KMessage::messages[msgname];
			}
		}
		else
		{
			//todo replace handler with real handle call back!
			KMessage* msg = new KMessage(msgid, (char*) msgname.c_str() , msglen, argstype, argstypes, NULL);
					
		/*	if(!isClientMethod)
				Dbg.DEBUG_MSG(string.Format("KBEngine::onImportClientMessages[{0}]: imported({1}/{2}/{3}) successfully!", 
					currserver_, msgname, msgid, msglen));*/
			std::string curserver_	= "loginapp";

			if( base_ip!="")
				curserver_ = "baseapp";
			//
			if(curserver_ == "loginapp")
				KMessage::loginappMessages[msgid] = msg;
			else
				KMessage::baseappMessages[msgid] = msg;
		}
	};

			onImportClientMessagesCompleted();
			//Todo: send next packet. loginapp hello
}


void KBEngineClient::KMessage::onImportEntityDefCompleted()
{
	//done
	login_baseapp(false);
}

void KBEngineClient::KMessage::onImportClientMessagesCompleted()
{
	//send ["Loginapp_importClientMessages"]);
	KBEngineClient::KBundle* bundle = new KBundle();
	KNetworkInterface* network = new KNetworkInterface();
	if ( base_ip =="")
		bundle->newmessage( *KMessage::messages["Loginapp_hello"] );
	else
		bundle->newmessage( *KMessage::messages["Baseapp_hello"] );
	std::string clientver = "0.1.7";
	//local server
	clientver = "0.1.5";
	bundle->writeString( clientver );
	uint8* clientdatas_ = new uint8[0];
	bundle->writeBlob((char*)clientdatas_,0);
	bundle->send(*network);
	delete bundle;

	if ( base_ip =="")
	{
		//importMercuryErrorDescr
		bundle = new KBundle();
		bundle->newmessage( *KMessage::messages["Loginapp_importMercuryErrorsDescr"]);
		bundle->send(*network);
		delete bundle;
		//after this  login_loginapp(false);
	
		bundle = new KBundle(); 
		bundle->newmessage( *KMessage::messages["Loginapp_login"]);
		int8 clientType = 5;
		bundle->writeInt8(clientType); // clientType
		bundle->writeBlob( new char[0], 0);
		std::string username = "kbe123";
		std::string password = "111111";
		bundle->writeString(username);
		bundle->writeString(password);
		bundle->send( *network );
	}
	else
	{
		//baseapp part.
		bundle = new KBundle();
		bundle->newmessage( *KMessage::messages["Baseapp_importClientEntityDef"]);
		bundle->send( *network );
		//Event.fireOut("Baseapp_importClientEntityDef", new object[]{});
	}
	//createAccount_loginapp(false);

	//import client entity def
	//Bundle bundle = new Bundle();
	//bundle.newMessage(Message.messages["Baseapp_importClientEntityDef"]);
	//bundle.send(networkInterface_);
	//Event.fireOut("Baseapp_importClientEntityDef", new object[]{});
	// 
	// on import done! 

}


void KBEngineClient::KMessage::Client_onHelloCB(MemoryStream& stream)
{
	std::string serverVersion_;
	stream >> serverVersion_ ;
	int32 ctype;
	stream >> ctype;
	CCLog ( "KBEngine::Client_onHelloCB: verInfo( %s ), ctype( %d )! " ,  serverVersion_.c_str()  , ctype  );
}

void KBEngineClient::KMessage::Client_onVersionNotMatch(MemoryStream& stream)
{
	std::string serverVersion_;
	stream>>serverVersion_;
	CCLog(" serverVersion is %s" ,serverVersion_.c_str());

}

void KBEngineClient::KMessage::Client_onLoginSuccessfully(MemoryStream& stream)
{
	std::string accountName;
	stream >> accountName;
	std::string  username = accountName;
	std::string ip;
	stream >> ip;
	uint16 port;
	stream>>port;

	std::string serverdatas_;
	stream >> serverdatas_;
			
	CCLog("KBEngine::Client_onLoginSuccessfully: accountName( %s ), addr( %s : port %d), datas( %d )!" , accountName.c_str() , 
			ip.c_str() ,  port , serverdatas_.length()  );
			
	base_ip = ip;
	base_port = port;
	login_baseapp(true);
}

void KBEngineClient::KMessage::Client_onImportMercuryErrorsDescr(MemoryStream& stream)
{
	uint16 size;
	stream>>size;
	while(size > 0)
	{
		size -= 1;
				
		//MercuryErr e;
		uint16 id;
		stream>>id;
		std::string name;
		stream>>name;
		std::string descr;
		stream>>descr;

		//e.id = stream.readUint16();
		//e.name = stream.readString();
		//e.descr = stream.readString();				
		//mercuryErrs.Add(e.id, e);
					
		CCLog("Client_onImportMercuryErrorsDescr: id= %d  name=%s descr=%s", id  , name.c_str() , descr.c_str() );
	}
}

void KBEngineClient::KMessage::Client_onLoginGatewayFailed(uint16 failedcode)
{
	CCLog("KBEngine::Client_onLoginGatewayFailed: failedcode( %d" ,  failedcode );
	//Event.fireOut("onLoginGatewayFailed", new object[]{failedcode});
}

void KBEngineClient::KMessage::Client_onCreatedProxies(MemoryStream& stream)
{
	//(UInt64 rndUUID, Int32 eid, string entityType)
	CCLog("Todo create entity of account!");
	//continue get various method called. update_properties..
	//Todo: how to decode stream to method call? with ScriptModule data? 
	
	//Test only we should create a test class and interoperation with server side .
	// send select char.
	// reqAccoutCharList. 
	// 
	uint64 rndUUID;
	int32 eid;
	string entityType;

	stream>>rndUUID;
	stream>>eid;
	stream>>entityType;

	KBEngineClient::ClientApp::getInstance().onCreatedProxies(rndUUID,eid, entityType);
	//
	CCLog ("KBEngine::Client_onCreatedProxies: eid( %d ) entityType( %s ) ", eid , entityType.c_str() );
	
	/*entity_uuid = rndUUID;
	entity_id = eid;
	entity_type = entityType;
			
	Type runclass = EntityDef.moduledefs[entityType].script;
	if(runclass == null)
		return;*/
			
	//Entity entity = (Entity)Activator.CreateInstance(runclass);
	KBEngineClient::Entity* entity;
	if ( entityType == "Account")
		entity =  (Entity*) ( new KBEngineClient::KAccount());
	if ( entityType == "Avatar")
		entity =  (Entity*) ( new KBEngineClient::KAvatar());
	//fake logic here.

	entity->id = eid;
	entity->classtype = entityType;
			
	entity->baseMailbox = new MailBox();
	entity->baseMailbox->id = eid;
	entity->baseMailbox->classtype = entityType;
	entity->baseMailbox->type = MAILBOX_TYPE_BASE;
			
	
	//entities[eid] = entity;
	KBEngineClient::ClientApp::getInstance().entities[eid] = entity;			
	entity->__init__();
	//
}

void KBEngineClient::KMessage::login_baseapp( bool noconnect )
{

	//in here, we should connect to baseapp with returned ip and port.
	if (noconnect)
	{
		//Reconnect to baseapp  
		KBEGameSocket& game_sock = KBEGameSocket::getInstance();
		//disconnect from loginapp 
		game_sock.connectionServer( base_ip.c_str() , base_port);
		//
		//onLogin_baseapp();
		//send request importClientMessages.
		KBundle* bundle = new KBundle();
		bundle->newmessage( *KMessage::messages["Baseapp_importClientMessages"]);
		KBEngineClient::KNetworkInterface* network = new KBEngineClient::KNetworkInterface();
		bundle->send( *network );
		CCLog("KBEngine::onLogin_baseapp: start importClientMessages ...");
		//Event.fireOut("Baseapp_importClientMessages", new object[]{});
	}else{
		//baseapp branch
		KBundle* bundle = new KBundle();
		bundle->newmessage( *KMessage::messages["Baseapp_loginGateway"]);
		std::string username = "kbe123";
		std::string password = "111111";
		bundle->writeString(username);
		bundle->writeString(password);
		KBEngineClient::KNetworkInterface* network = new KBEngineClient::KNetworkInterface();
		bundle->send( *network );
	
	}
}


			
//huge packet be parsed here, store entity define data .
void KBEngineClient::KMessage::onImportClientEntityDef( MemoryStream& stream )
{
		//52 
		uint16 aliassize = stream.readUint16();
		printf("KBEngine::Client_onImportClientEntityDef: importAlias(size= %d )!",  aliassize );
			
		while(aliassize > 0)
		{
			aliassize--;
			createDataTypeFromStream(stream, true);
		};
		
		//after this, rpos of stream should be changed. if not use & or pointer. stream will never be changed.
		
		/*	foreach(string datatype in EntityDef.datatypes.Keys)
			{
				if(EntityDef.datatypes[datatype] != null)
				{
					EntityDef.datatypes[datatype].bind();
				}
			}*/
			//when quit loop?
			for( std::map<std::string,KBEDATATYPE_BASE>::iterator it =  EntityDef::datatypes.begin(); it != EntityDef::datatypes.end(); it++)
			{
				//if( it->second)
				CCLog("datatypes  %s  %s", it->first.c_str() ,  it->second.name.c_str()  );
				continue;
				//
				if( it->second.name !="" )
					  it->second.bind();
				else
					CCLog("datatypes  %s not binded", it->first.c_str() );
			}
			//bind datatype class.

			while(stream.opsize() > 0)
			{
				string scriptmethod_name = stream.readString();
				uint16 scriptUtype = stream.readUint16();
				uint16 propertysize = stream.readUint16();
				uint16 methodsize = stream.readUint16();
				uint16 base_methodsize = stream.readUint16();
				uint16 cell_methodsize = stream.readUint16();
				
				printf("KBEngine::Client_onImportClientEntityDef: import( %s ), propertys( %d ) clientMethods( %d ), baseMethods( %d ), cellMethods( %d )!",
					scriptmethod_name.c_str() , propertysize , methodsize , base_methodsize ,  cell_methodsize );
				
				
				ScriptModule* module =  new ScriptModule(scriptmethod_name);
				//KBEngineClient::EntityDef::moduledefs[scriptmethod_name] = *module;
				//EntityDef::idmoduledefs[scriptUtype] = *module;
				
				//Dictionary<string, Property> defpropertys = new Dictionary<string, Property>();
				PropertyMap defpropertys;
				EntityDef::alldefpropertys[scriptmethod_name] = & defpropertys;
				
				//Todo: with cpp version reflection it.
				//Type Class = module.script;
				
				while(propertysize > 0)
				{
					propertysize--;
					
					uint16 properUtype = stream.readUint16();
					int16 ialiasID = stream.readInt16();
					string name = stream.readString();
					string defaultValStr = stream.readString();
					KBEDATATYPE_BASE& utype = ( EntityDef::iddatatypes[stream.readUint16()] );
					
					//System.Reflection.MethodInfo setmethod = null;
					
					/*if(Class != null)
					{
						setmethod = Class.GetMethod("set_" + name);
					}*/

					std::string setmethodName =  "set_"+name;
					
					pMethod setmethod = 0;

					Property savedata = * ( new Property() );
					savedata.name = name;
					savedata.properUtype = properUtype;
					savedata.aliasID = ialiasID;
					savedata.defaultValStr = defaultValStr;
					savedata.utype = utype;
					savedata.setmethod = setmethodName;
					
					module->propertys[name] = &savedata;
					
					if(ialiasID >= 0)
					{
						module->usePropertyDescrAlias = true;
						module->idpropertys[(uint16)ialiasID] = &savedata;
					}
					else
					{
						module->usePropertyDescrAlias = false;
						module->idpropertys[properUtype] = &savedata;
					}

					//Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), property(" + name + "/" + properUtype + ").");
				};
				
				while(methodsize > 0)
				{
					methodsize--;
					
					uint16 methodUtype = stream.readUint16();
					int16 ialiasID = stream.readInt16();
					string name = stream.readString();
					Byte argssize = stream.readUint8();
					//List<KBEDATATYPE_BASE> args = new List<KBEDATATYPE_BASE>();
					std::vector<KBEDATATYPE_BASE*> args;
					while(argssize > 0)
					{
						argssize--;
						//args.Add(EntityDef.iddatatypes[stream.readUint16()]);
						args.push_back( &EntityDef::iddatatypes[stream.readUint16()] );
					};
					
					Method* savedata_ = new Method();
					Method& savedata = *savedata_;
					savedata.name = name;
					savedata.methodUtype = methodUtype;
					savedata.aliasID = ialiasID;
					savedata.args = args;
					
					/*if(Class != null)
						savedata.handler = Class.GetMethod(name);*/
							
					module->methods[name] = savedata_;
					
					if(ialiasID >= 0)
					{
						module->useMethodDescrAlias = true;
						module->idmethods[(uint16)ialiasID] =savedata_;
					}
					else
					{
						module->useMethodDescrAlias = false;
						module->idmethods[methodUtype] = savedata_;
					}
					
					printf ("KBEngine::Client_onImportClientEntityDef: add( %s ), method( %s  )." , scriptmethod_name.c_str() , name.c_str() );
				};
	
				while(base_methodsize > 0)
				{
					base_methodsize--;
					
					uint16 methodUtype = stream.readUint16();
					int16 ialiasID = stream.readInt16();
					string name = stream.readString();
					Byte argssize = stream.readUint8();
					//List<KBEDATATYPE_BASE> args = new List<KBEDATATYPE_BASE>();
					std::vector<KBEDATATYPE_BASE*> args;

					while(argssize > 0)
					{
						argssize--;
						//args.Add(EntityDef.iddatatypes[stream.readUint16()]);
						args.push_back( &EntityDef::iddatatypes[stream.readUint16()] );
					};
					
					Method* savedata_  = new Method();
					Method& savedata = *savedata_;
					savedata.name = name;
					savedata.methodUtype = methodUtype;
					savedata.aliasID = ialiasID;
					savedata.args = args;
					
					module->base_methods[name] = savedata_;
					module->idbase_methods[methodUtype] = savedata_;
					
					printf ("KBEngine::Client_onImportClientEntityDef: add( %s ), base_method( %s)." , scriptmethod_name.c_str() , name.c_str() );
				};
				
				while(cell_methodsize > 0)
				{
					cell_methodsize--;
					
					uint16 methodUtype = stream.readUint16();
					int16 ialiasID = stream.readInt16();
					string name = stream.readString();
					Byte argssize = stream.readUint8();
					//List<KBEDATATYPE_BASE> args = new List<KBEDATATYPE_BASE>();
					std::vector<KBEDATATYPE_BASE*> args;
					while(argssize > 0)
					{
						argssize--;
						//args.Add(EntityDef.iddatatypes[stream.readUint16()]);
						args.push_back( &EntityDef::iddatatypes[stream.readUint16()] );
						//stored a decode handler? 2014-06-27 to confirm it.
					};
					
					Method* savedata_  = new Method();
					Method& savedata = *savedata_;
					savedata.name = name;
					savedata.methodUtype = methodUtype;
					savedata.aliasID = ialiasID;
					savedata.args = args;
				
					module->cell_methods[name] = savedata_;
					module->idcell_methods[methodUtype] = savedata_;
					printf ("KBEngine::Client_onImportClientEntityDef: add( %s ), cell_method( %s)." , scriptmethod_name.c_str() , name.c_str() );
	
					//Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), cell_method(" + name + ").");
				};
				
				//if(module.script == null)
				//{
				//	Dbg.ERROR_MSG("KBEngine::Client_onImportClientEntityDef: module(" + scriptmethod_name + ") not found!");
				//}
					
				//foreach(string name in module.propertys.Keys)
				for( PropertyMapItr it = module->propertys.begin(); it != module->propertys.end(); it ++)
				{
				
					Property infos = *it->second;//module->propertys[name];
					
					Property newp  = * (new Property() );
					newp.name = infos.name;
					newp.properUtype = infos.properUtype;
					newp.aliasID = infos.aliasID;
					newp.utype = infos.utype;
					newp.val = infos.utype.parseDefaultValStr(infos.defaultValStr);
					newp.setmethod = infos.setmethod;
					
					//defpropertys.Add(infos.name, newp);
					defpropertys[ infos.name ]  = &newp;

					//if(module.script != null && module.script.GetMember(name) == null)
					//{
					//	Dbg.ERROR_MSG(scriptmethod_name + "(" + module.script + "):: property(" + name + ") no defined!");
					//}
				};
	
				//foreach(string name in module.methods.Keys)
				for(std::map<std::string,Method*>::iterator it = module->methods.begin(); it != module->methods.end(); it ++)
 				{
					Method infos = *it->second; //module->methods[name];

				/*	if(module.script != null && module.script.GetMethod(name) == null)
					{
						Dbg.WARNING_MSG(scriptmethod_name + "(" + module.script + "):: method(" + name + ") no implement!");
					}*/
				};

				//here store 
				KBEngineClient::EntityDef::moduledefs[scriptmethod_name] = *module;
				EntityDef::idmoduledefs[scriptUtype] = *module;
			}
			
			onImportEntityDefCompleted();
}

void KBEngineClient::KMessage::createDataTypeFromStream( MemoryStream& stream, bool canprint )
{
	//Read Data Alias and stored it.
	uint16 utype = stream.readUint16();
	std::string name = stream.readString();
	std::string valname = stream.readString();
			
	if(canprint)
		CCLog("KBEngine::Client_onImportClientEntityDef: importAlias( %s : %s )! ", name.c_str() , valname.c_str() );
			
	if(valname == "FIXED_DICT")
	{
		//AvatarInfo need this data type.
		KBEDATATYPE_FIXED_DICT datatype = *( new KBEDATATYPE_FIXED_DICT() );
		Byte keysize = stream.readUint8();
		datatype.implementedBy = stream.readString();
		datatype.name = valname ;
		while(keysize > 0)
		{
			keysize--;
					
			string keyname = stream.readString();
			uint16 keyutype = stream.readUint16();
			datatype.dicttype[keyname] = keyutype;
			CCLog("EntityDef::datatypes key %s stored  %d", keyname.c_str() ,  keyutype );

		};
		//here is to store a data 
		//KBEDATATYPE_FIXED_DICT d =  datatype;
		EntityDef::datatypes[name] = (KBEDATATYPE_BASE*) &datatype; //missed something?
		CCLog("EntityDef::datatypes %s stored  type= %s", name.c_str() ,datatype.name.c_str());
	}
	else if(valname == "ARRAY")
	{
		uint16 uitemtype = stream.readUint16();
		KBEDATATYPE_ARRAY datatype = *(new KBEDATATYPE_ARRAY() );
		//datatype.type = uitemtype;
		datatype.name = valname;
		EntityDef::datatypes[name] = (KBEDATATYPE_BASE*) &datatype;
	}
	else
	{
		//KBEDATATYPE_BASE val;
		//EntityDef::datatypes.TryGetValue(valname, out val);
		//EntityDef::datatypes[name] = val;
	}
	
	EntityDef::iddatatypes[utype] = EntityDef::datatypes[name];
	EntityDef::datatype2id[name] = EntityDef::datatype2id[valname];

	//EntityDef.iddatatypes[utype] = EntityDef.datatypes[name];
	//EntityDef.datatype2id[name] = EntityDef.datatype2id[valname];
}

void KBEngineClient::KMessage::sendActiveAck()
{
	
	std::map<std::string,KMessage*>::iterator it = KMessage::messages.find("Loginapp_onClientActiveTick");
	//
	std::map<std::string,KMessage*>::iterator it2 = KMessage::messages.find("Baseapp_onClientActiveTick");

 	KBundle* bundle = new KBundle();
	if( base_ip =="") //currserver_ == "loginapp")
	{
		if( it == KMessage::messages.end() )
			return;
		bundle->newmessage( *KMessage::messages["Loginapp_onClientActiveTick"]);
	}
	else
	{
		if( it2 == KMessage::messages.end())
			return;
		bundle->newmessage( *KMessage::messages["Baseapp_onClientActiveTick"]);
	}
	KBEngineClient::KNetworkInterface* network = new KBEngineClient::KNetworkInterface();	
	bundle->send( *network );
	delete bundle;
}

void KBEngineClient::KMessage::Client_onRemoteMethodCall( MemoryStream& stream )
{
	ENTITY_ID eid = 0;
	stream >> eid;

	KBEngineClient::Entity* entity = KBEngineClient::ClientApp::getInstance().entities.find(eid)->second;
	uint16 methodUtype = 0;

	uint8  methodUtype1 = 0;
	if(EntityDef::moduledefs[entity->classtype].useMethodDescrAlias)
		//methodUtype1 = stream.readUint8();
		stream >> methodUtype1;
	else
		//methodUtype = stream.readUint16();
		stream >> methodUtype;
	
	if( methodUtype1 !=0 )
		methodUtype = methodUtype1;
	//
	
	Method& methoddata = *EntityDef::moduledefs[entity->classtype].idmethods[methodUtype];
			
	CCLog ("KBEngine::Client_onRemoteMethodCall: %s, %s " , entity->classtype.c_str() , methoddata.name.c_str() );
	
	object* args = new object [ methoddata.args.size() ];


	//object[] args = new object[methoddata.args.Count];
	if( methoddata.name == "onReqAvatarList")
	{
		for(int i=0; i<methoddata.args.size(); i++)
		{
			//args[i] = methoddata.args[i].createFromStream(stream);
			CCLog( methoddata.args[i]->name.c_str()  ); 
		}
	}		
	//methoddata.handler.Invoke(entity, args);

	entity->onRemoteMethodCall(methoddata.name,stream);
}
