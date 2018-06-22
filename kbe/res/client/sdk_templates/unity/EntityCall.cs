namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
	/*
		实体的EntityCall
		关于EntityCall请参考API手册中对它的描述
		https://github.com/kbengine/kbengine/tree/master/docs/api
	*/
    public class EntityCall
    {
    	// EntityCall的类别
		public enum ENTITYCALL_TYPE
		{
			ENTITYCALL_TYPE_CELL = 0,		// CELL_ENTITYCALL_TYPE
			ENTITYCALL_TYPE_BASE = 1		// BASE_ENTITYCALL_TYPE
		}
		
    	public Int32 id = 0;
		public string className = "";
		public ENTITYCALL_TYPE type = ENTITYCALL_TYPE.ENTITYCALL_TYPE_CELL;
		
		public Bundle bundle = null;
		
		public EntityCall(Int32 eid, string ename)
		{
			id = eid;
			className = ename;
		}
		
		public virtual void __init__()
		{
		}
		
		public virtual bool isBase()
		{
			return type == ENTITYCALL_TYPE.ENTITYCALL_TYPE_BASE;
		}
	
		public virtual bool isCell()
		{
			return type == ENTITYCALL_TYPE.ENTITYCALL_TYPE_CELL;
		}
		
		/*
			创建新的mail
		*/
		public Bundle newCall()
		{  
			if(bundle == null)
				bundle = Bundle.createObject();
			
			if(isCell())
				bundle.newMessage(Messages.messages["Baseapp_onRemoteCallCellMethodFromClient"]);
			else
				bundle.newMessage(Messages.messages["Entity_onRemoteMethodCall"]);
	
			bundle.writeInt32(this.id);
			
			return bundle;
		}
		
		/*
			向服务端发送这个mail
		*/
		public void sendCall(Bundle inbundle)
		{
			if(inbundle == null)
				inbundle = bundle;
			
			inbundle.send(KBEngineApp.app.networkInterface());
			
			if(inbundle == bundle)
				bundle = null;
		}

		public Bundle newCall(string methodName, UInt16 entitycomponentPropertyID = 0)
		{			
			if(KBEngineApp.app.currserver == "loginapp")
			{
				Dbg.ERROR_MSG(className + "::newCall(" + methodName + "), currserver=!" + KBEngineApp.app.currserver);  
				return null;
			}

			ScriptModule module = null;
			if(!EntityDef.moduledefs.TryGetValue(className, out module))
			{
				Dbg.ERROR_MSG(className + "::newCall: entity-module(" + className + ") error, can not find from EntityDef.moduledefs");
				return null;
			}
				
			Method method = null;

			if(isCell())
			{
				method = module.cell_methods[methodName];
			}
			else
			{
				method = module.base_methods[methodName];
			}

			UInt16 methodID = method.methodUtype;

			newCall();
			
			bundle.writeUint16(entitycomponentPropertyID);
			bundle.writeUint16(methodID);
			return bundle;
		}
    }
    
} 
