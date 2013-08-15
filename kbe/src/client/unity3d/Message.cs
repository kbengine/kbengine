namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
	using MessageID = System.UInt16;
	
    public class Message 
    {
    	public MessageID id = 0;
		public string name;
		public Int16 msglen = -1;
		public System.Reflection.MethodInfo handler = null;
		public List<Byte> argtypes = null;
		
		public static Dictionary<MessageID, Message> loginappMessages = new Dictionary<MessageID, Message>();
		public static Dictionary<MessageID, Message> baseappMessages = new Dictionary<MessageID, Message>();
		public static Dictionary<MessageID, Message> clientMessages = new Dictionary<MessageID, Message>();
		
		public static Dictionary<string, Message> messages = new Dictionary<string, Message>();
		
		public Message(MessageID msgid, string msgname, Int16 length, List<Byte> msgargtypes, System.Reflection.MethodInfo msghandler)
		{
			id = msgid;
			name = msgname;
			msglen = length;
			handler = msghandler;
			argtypes = msgargtypes;
			
			// Debug.Log(string.Format("Message::Message(): ({0}/{1}/{2})!", 
			//	msgname, msgid, msglen));
		}
		
		public object[] createFromStream(MemoryStream msgstream)
		{
			return new object[]{msgstream};
		}
		
		public void handleMessage(MemoryStream msgstream)
		{
			if(argtypes.Count <= 0)
				handler.Invoke(KBEngineApp.app, new object[]{msgstream});
			else
				handler.Invoke(KBEngineApp.app, createFromStream(msgstream));
		}
    }
} 
