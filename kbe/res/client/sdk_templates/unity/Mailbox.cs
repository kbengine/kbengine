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
    public class Mailbox 
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
		
		private NetworkInterface networkInterface_;
		
		public Bundle bundle = null;
		
		public Mailbox()
		{
			networkInterface_ = KBEngineApp.app.networkInterface();
		}
		
		public virtual void __init__()
		{
		}
		
		bool isBase()
		{
			return type == MAILBOX_TYPE.MAILBOX_TYPE_BASE;
		}
	
		bool isCell()
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
			
			if(type == Mailbox.MAILBOX_TYPE.MAILBOX_TYPE_CELL)
				bundle.newMessage(Message.messages["Baseapp_onRemoteCallCellMethodFromClient"]);
			else
				bundle.newMessage(Message.messages["Base_onRemoteMethodCall"]);
	
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
			
			inbundle.send(networkInterface_);
			
			if(inbundle == bundle)
				bundle = null;
		}
    }
    
} 
