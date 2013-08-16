namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
    public class Mailbox 
    {
		public enum MAILBOX_TYPE
		{
			MAILBOX_TYPE_CELL = 0,
			MAILBOX_TYPE_BASE = 1
		}
		
    	public Int32 id = 0;
		public string classtype = "";
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
		
		public Bundle newMail()
		{  
			if(bundle == null)
				bundle = new Bundle();
			
			if(type == Mailbox.MAILBOX_TYPE.MAILBOX_TYPE_CELL)
				bundle.newMessage(Message.messages["Baseapp_onRemoteCallCellMethodFromClient"]);
			else
				bundle.newMessage(Message.messages["Base_onRemoteMethodCall"]);
	
			bundle.writeInt32(this.id);
			
			return bundle;
		}
		
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
