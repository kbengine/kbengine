namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
	/*
		实体的Mailbox
		关于Mailbox请参考API手册中对它的描述
		https://github.com/kbengine/kbengine/tree/master/docs/api
	*/
    public class EntityMailbox 
    {
    	// Mailbox的类别
		public enum MAILBOX_TYPE
		{
			MAILBOX_TYPE_CELL = 0,		// CELL_MAILBOX
			MAILBOX_TYPE_BASE = 1		// BASE_MAILBOX
		}
		
    	public Int32 id = 0;
		public string className = "";
		public MAILBOX_TYPE type = MAILBOX_TYPE.MAILBOX_TYPE_CELL;
		
		public Bundle bundle = null;
		
		public EntityMailbox()
		{
		}
		
		public virtual void __init__()
		{
		}
		
		public virtual bool isBase()
		{
			return type == MAILBOX_TYPE.MAILBOX_TYPE_BASE;
		}
	
		public virtual bool isCell()
		{
			return type == MAILBOX_TYPE.MAILBOX_TYPE_CELL;
		}
		
		/*
			创建新的mail
		*/
		public Bundle newMail()
		{  
			if(bundle == null)
				bundle = Bundle.createObject();
			
			if(type == EntityMailbox.MAILBOX_TYPE.MAILBOX_TYPE_CELL)
				bundle.newMessage(Messages.messages["Baseapp_onRemoteCallCellMethodFromClient"]);
			else
				bundle.newMessage(Messages.messages["Base_onRemoteMethodCall"]);
	
			bundle.writeInt32(this.id);
			
			return bundle;
		}
		
		/*
			向服务端发送这个mail
		*/
		public void postMail(Bundle inbundle)
		{
			if(inbundle == null)
				inbundle = bundle;
			
			inbundle.send(KBEngineApp.app.networkInterface());
			
			if(inbundle == bundle)
				bundle = null;
		}
    }
    
} 
