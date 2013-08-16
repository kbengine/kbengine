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
		public System.Reflection.MethodInfo[] argtypes = null;
		
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
			
			argtypes = new System.Reflection.MethodInfo[msgargtypes.Count];
			for(int i=0; i<msgargtypes.Count; i++)
			{
				argtypes[i] = StreamRWBinder.bindReader(msgargtypes[i]);
				if(argtypes[i] == null)
				{
					Debug.LogError("Message::Message(): bindReader(" + msgargtypes[i] + ") is error!");
				}
			}
			
			// Debug.Log(string.Format("Message::Message(): ({0}/{1}/{2})!", 
			//	msgname, msgid, msglen));
		}
		
		public object[] createFromStream(MemoryStream msgstream)
		{
			if(argtypes.Length <= 0)
				return new object[]{msgstream};
			
			object[] result = new object[argtypes.Length];
			
			for(int i=0; i<argtypes.Length; i++)
			{
				result[i] = argtypes[i].Invoke(msgstream, new object[0]);
			}
			
			return result;
		}
		
		public void handleMessage(MemoryStream msgstream)
		{
			if(argtypes.Length <= 0)
				handler.Invoke(KBEngineApp.app, new object[]{msgstream});
			else
				handler.Invoke(KBEngineApp.app, createFromStream(msgstream));
		}
    }
} 
