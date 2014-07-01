#pragma once

#include "KMailbox.h"
#include "KDataTypes.h"
#include "KEntitydef.h"

namespace KBEngineClient
{
	class ClientApp;
	//the base game object
	class Entity{
	public:
		int32 id;
		std::string classtype;
		Vector3 position;// = new Vector3(0.0f, 0.0f, 0.0f);
		Vector3 direction;// = new Vector3(0.0f, 0.0f, 0.0f);
		float velocity;// = 0.0f;
		
		bool isOnGround;// = true;
		
		//public object renderObj = null;
		
		MailBox* baseMailbox;// = null;
		MailBox* cellMailbox;// = null;
		
		bool inWorld;// = false;
		PropertyMap alldefpropertys;
		PropertyMap defpropertys_;
		PropertyIdMap iddefpropertys_;
		//public static Dictionary<string, Dictionary<string, Property>> alldefpropertys = 
		//	new Dictionary<string, Dictionary<string, Property>>();
		//
		//private Dictionary<string, Property> defpropertys_ = 
		//	new Dictionary<string, Property>();
		//
		//private Dictionary<UInt16, Property> iddefpropertys_ = 
		//	new Dictionary<UInt16, Property>();
		
		Entity():inWorld(false),id(0),velocity(0.0f),isOnGround(true)
		{
			/*Dictionary<string, Property> datas = alldefpropertys[GetType().Name];
			foreach(Property e in datas.Values)
			{
				Property newp = new Property();
				newp.name = e.name;
				newp.properUtype = e.properUtype;
				newp.utype = e.utype;
				newp.val = e.val;
				newp.setmethod = e.setmethod;
				defpropertys_.Add(e.name, newp);
				iddefpropertys_.Add(e.properUtype, newp);
			}*/
		}

		virtual void onUpdatePropertys(MemoryStream& s){};
		
		virtual void destroy ()
		{
		}
		
		bool isPlayer();
		
		
		void addDefinedPropterty(string name, object v)
		{
			Property newp;// = new Property();
			newp.name = name;
			newp.properUtype = 0;
			newp.val = v;
			newp.setmethod = "";
			//defpropertys_.Add(name, newp);
			defpropertys_[name] = &newp;
		}
		
		object getDefinedPropterty(string name)
		{
			//return 0;
			//Property obj = null;
			//if(!defpropertys_.TryGetValue(name, out obj))
			//{
			//	return null;
			//}
			PropertyMapItr it = defpropertys_.find(name);
			if ( it != defpropertys_.end())
				return defpropertys_[name]->val;
			//return defpropertys_[name].val;
            return 0;
		}
		
		void setDefinedPropterty(string name, object val)
		{
			defpropertys_[name]->val = val;
		}
		
		object getDefinedProptertyByUType(uint16 utype)
		{
			PropertyIdMapItr it = iddefpropertys_.find(utype);
			if ( it != iddefpropertys_.end())
				return iddefpropertys_[utype]->val;
			//return 0;
			/*Property obj = null;
			if(!iddefpropertys_.TryGetValue(utype, out obj))
			{
				return null;
			}
			
			return iddefpropertys_[utype].val;*/
            return 0;
		}
		
		void setDefinedProptertyByUType(uint16 utype, object val)
		{
			iddefpropertys_[utype]->val = val;
		}
		
		virtual void __init__()
		{
		}

		virtual void baseCall(string methodname, MemoryStream& stream)
		{
			Method& method = * EntityDef::moduledefs[classtype].base_methods[methodname];
			uint16 methodID = method.methodUtype;
			baseMailbox->newMail();
			baseMailbox->bundle->writeUint16(methodID);
			if( stream.opsize() >0 ) //no paramter works. bugs when has paramter. 
				baseMailbox->bundle->writeStream( &stream );
			baseMailbox->postMail(0);
		}
		virtual void baseCall(string methodname, int arguments, ...)
		{	
			
			CCAssert(false," deprecated basecall!! ");
			return;

			Method& method =  * EntityDef::moduledefs[classtype].base_methods[methodname];
			uint16 methodID = method.methodUtype;
			
			//if(arguments->Length != method.args.Count)
			//{
			//	//Dbg.ERROR_MSG("Entity::baseCall: args(" + (arguments.Length) + "!= " + method.args.Count + ") size is error!");  
			//	return;
			//}
			//} 

			if( arguments != method.args.size())
			{
				printf(" Entity::baseCall: args( %d != %lu ) size is error!", arguments,method.args.size());
				return;
			}
			
			baseMailbox->newMail();
			baseMailbox->bundle->writeUint16(methodID);
			
			// if arg[0] is int writeInt. ... and the like.
			// how to judge its type and convert to stream or directlly call  <<stream
			MemoryStream stream;

			int nSum = 0;
			int* p = &arguments;
			for(int i=0; i<arguments; ++i)
			{
				//get format description and according it call which function ?
				stream << *(++p) ;
				//
				std::string rolename;
				stream >> rolename;
			}

			if( arguments > 0)
				baseMailbox->bundle->writeStream( &stream );
			//we need check arguments type and store it to stream.
			//try
			//{
			//	for(var i=0; i<method.args.Count; i++)
			//	{
			//		method.args[i].addToStream(baseMailbox.bundle, arguments[i]);
			//	}
			//}
			//catch(Exception e)
			//{
			//	//Dbg.ERROR_MSG("Entity::baseCall: args is error(" + e.Message + ")!");  
			//	baseMailbox->bundle = 0;
			//	return;
			//}
			
			baseMailbox->postMail(0);
		}
		
		virtual void cellCall(string methodname, object arguments,...)
		{
			Method& method = *EntityDef::moduledefs[classtype].cell_methods[methodname];
			uint16 methodID = method.methodUtype;
			
			//if(arguments.Length != method.args.Count)
			//{
			//	//Dbg.ERROR_MSG("Entity::cellCall: args(" + (arguments.Length) + "!= " + method.args.Count + ") size is error!");  
			//	return;
			//}

			cellMailbox->newMail();
			cellMailbox->bundle->writeUint16(methodID);
				
		/*	try
			{
				for(var i=0; i<method.args.Count; i++)
				{
					method.args[i].addToStream(cellMailbox.bundle, arguments[i]);
				}
			}
			catch(Exception e)
			{
				Dbg.ERROR_MSG("Entity::cellCall: args is error(" + e.Message + ")!");  
				cellMailbox.bundle = null;
				return;
			}*/

			cellMailbox->postMail(0);
		}
	
		virtual void enterWorld()
		{
			//Dbg.DEBUG_MSG(classtype + "::enterWorld(" + getDefinedPropterty("uid") + "): " + id); 
			inWorld = true;
			//Event.fireOut("onEnterWorld", new object[]{this});
		}
		
		virtual void leaveWorld()
		{
			//Dbg.DEBUG_MSG(classtype + "::leaveWorld: " + id); 
			inWorld = false;
			//Event.fireOut("onLeaveWorld", new object[]{this});
		}
		
		virtual void set_position(object old)
		{
			//Vector3 v = (Vector3)getDefinedPropterty("position");
			//position = v;
			//Dbg.DEBUG_MSG(classtype + "::set_position: " + old + " => " + v); 
			
			//if(isPlayer())
			//	KBEngineApp.app.entityServerPos = position;
			
			//Event.fireOut("set_position", new object[]{this});
		}

		virtual void set_direction(object old)
		{
			//Vector3 v = (Vector3)getDefinedPropterty("direction");
			
		/*	v.x = v.x * 360 / ((float)System.Math.PI * 2);
			v.y = v.y * 360 / ((float)System.Math.PI * 2);
			v.z = v.z * 360 / ((float)System.Math.PI * 2);
			
			direction = v;
			
			Dbg.DEBUG_MSG(classtype + "::set_direction: " + old + " => " + v); 
			Event.fireOut("set_direction", new object[]{this});*/
		}
		virtual void onRemoteMethodCall( std::string methodname, MemoryStream& s )
		{
			//
		}
	};

};

