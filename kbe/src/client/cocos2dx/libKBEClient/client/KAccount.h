#pragma once

#include	"KEntity.h"
#include	"KEntitydef.h"
#include	"KMessage.h"

namespace KBEngineClient
{
	typedef struct AvatarInfo_{
		uint64 dbid;
		std::string name;
		uint8	roletype;
		uint16	level;
	} AvatarInfo;

	class	KAccount : public Entity
	{
	public:
		//AvatarInfo List.
		std::map<uint64, AvatarInfo> avatars;

 		void onCreateAccountResult(int retcode){
			cocos2d::CCLog(" Account::onCreateAccountResult %d", retcode);		
		};

		//CLIENT_MESSAGE_HANDLER_STREAM Account,onReqAvatarList,int8 count;

		virtual void __init__()
		{
			MemoryStream s;
			baseCall("reqAvatarList", s);
			//
			//baseCall("reqCreateAvatar", 1, "cnsoft"+ this->id);
		}

		void onUpdatePropertys(MemoryStream& s)
		{
			cocos2d::CCLog("todo update account propertys from server.");

		};

		void onRemoteMethodCall( std::string methodname,MemoryStream& s )
		{
			if( methodname == "receiveMsg")
			{
				string who;
				string msg;
				s.readBlob ( who );
				s.readBlob ( msg );

				this->receiveMsg( who, msg ) ;
				//
			}

			if( methodname == "onCreateAvatarResult")
			{
				Byte retcode;
				s >> retcode;
				//
				AvatarInfo avatar;
				s >>avatar.dbid;
				//s >>_name; //how to read unicode string? use readBlob.
				s.readBlob( avatar.name);
				s >> avatar.roletype;
				s >> avatar.level;

				this->onCreateAvatarResult(retcode,avatar);
			}

			if( methodname == "onReqAvatarList")
			{
				//how to decode it? AVATAR_INFOS_LIST
				
				std::map<uint64,AvatarInfo> avatarInfos;
				//e.g: got argssize from entityDef. loop and got data.
				std::vector<uint64> dbids; 
				uint64 _dbid;
				string _name;
				uint8 _roletype;
				uint16 _level;
				
				uint32 size = s.readUint32();
				//cocos2d::CCLog(" args size = %d", size); //should read size. to move pointer position.
				while( s.opsize() >0 ){
					s >>_dbid;
					//s >>_name; //how to read unicode string? use readBlob.
					s.readBlob(_name);
					s >>_roletype;
					s >>_level;
 					cocos2d::CCLog(" got avatar info: %d %s %d %d",_dbid, _name.c_str(), _roletype,_level );
					AvatarInfo avatar;
					avatar.dbid = _dbid;
					avatar.name = _name;
					avatar.roletype = _roletype;
					avatar.level = _level;
					avatarInfos[_dbid] = avatar;
				}

				this->onReqAvatarList( avatarInfos );
			
			}
		
		}

		void onCreateAvatarResult(Byte retcode, AvatarInfo info)
		{
			if(retcode == 0)
			{
				//avatars.Add((UInt64)((Dictionary<string, object>)info)["dbid"], (Dictionary<string, object>)info);
				cocos2d::CCLog("Account::onCreateAvatarResult: ");//name= %s " + (string)((Dictionary<string, object>)info)["name"]);
			}
			else
			{
				cocos2d::CCLog("Account::onCreateAvatarResult: retcode=" + retcode);
			}

		}
		
		void onRemoveAvatar(uint64 dbid)
		{
		}
		
		void onReqAvatarList(std::map<uint64,AvatarInfo>& infos)
		{
	   		avatars.clear();
			//
			//List<object> listinfos = (List<object>)infos["values"];
			//	
			cocos2d::CCLog("Account::onReqAvatarList be called! ");
			
			for( int i =0 ;i < infos.size(); i++) 
			{
				
			}

			avatars = infos; 

			this->sendMsg(" cocos2dx here! ");
			//for test, we alsways select first character. request game.
#ifdef AUTO_LOGIN
			this->selectAvatarGame( avatars[5].dbid);
#endif	
		}
		
		void reqCreateAvatar(Byte roleType, string name)
		{
			//Dbg.DEBUG_MSG("Account::reqCreateAvatar: roleType=" + roleType);
			//baseCall("reqCreateAvatar",2, roleType, name);
			MemoryStream stream;
			stream << roleType;
			stream << name;
			baseCall("reqCreateAvatar",stream);
			//
		}

		void reqRemoveAvatar(string name)
		{
			//Dbg.DEBUG_MSG("Account::reqRemoveAvatar: name=" + name);
			//baseCall("reqRemoveAvatar",1, name);
		}
		
		void selectAvatarGame(uint64 dbid)
		{
			cocos2d::CCLog("Account::selectAvatarGame: dbid= %d" , dbid);
			//baseCall("selectAvatarGame",1, dbid);
			MemoryStream s;
			s << dbid;
			baseCall( "selectAvatarGame", s);
		}
		void sendMsg(string msg){
			MemoryStream s;
			//s<<msg;
			//unicode string. should write size 4byte first.
			s.appendBlob( msg.c_str() , msg.length());
			//
			baseCall("sendMsg",s);
		}
		void receiveMsg(string who, string msg)
		{
			cocos2d::CCLog(" receivemsg from %s , he say: %s ", who.c_str(), msg.c_str() );
		}
	
	};//end class declaration
};