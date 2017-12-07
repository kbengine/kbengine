namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
	using MessageID = System.UInt16;
	
	/*
		消息模块
		客户端与服务端交互基于消息通讯， 任何一个行为一条指令都是以一个消息包来描述
	*/
    public class Message 
    {
    	public MessageID id = 0;
		public string name;
		public Int16 msglen = -1;
		public System.Reflection.MethodInfo handler = null;
		public KBEDATATYPE_BASE[] argtypes = null;
		public sbyte argsType = 0;
			
		public static Dictionary<MessageID, Message> loginappMessages = new Dictionary<MessageID, Message>();
		public static Dictionary<MessageID, Message> baseappMessages = new Dictionary<MessageID, Message>();
		public static Dictionary<MessageID, Message> clientMessages = new Dictionary<MessageID, Message>();
		
		public static Dictionary<string, Message> messages = new Dictionary<string, Message>();

		public static void clear()
		{
			loginappMessages = new Dictionary<MessageID, Message>();
			baseappMessages = new Dictionary<MessageID, Message>();
			clientMessages = new Dictionary<MessageID, Message>();
			messages = new Dictionary<string, Message>();
		
			bindFixedMessage();
		}

		/*
			提前约定一些固定的协议
			这样可以在没有从服务端导入协议之前就能与服务端进行握手等交互。
		*/
		public static void bindFixedMessage()
		{
			// 引擎协议说明参见: http://www.kbengine.org/cn/docs/programming/clientsdkprogramming.html
			Message.messages["Loginapp_importClientMessages"] = new Message(5, "importClientMessages", 0, 0, new List<Byte>(), null);
			Message.messages["Loginapp_hello"] = new Message(4, "hello", -1, -1, new List<Byte>(), null);
			
			Message.messages["Baseapp_importClientMessages"] = new Message(207, "importClientMessages", 0, 0, new List<Byte>(), null);
			Message.messages["Baseapp_importClientEntityDef"] = new Message(208, "importClientMessages", 0, 0, new List<Byte>(), null);
			Message.messages["Baseapp_hello"] = new Message(200, "hello", -1, -1, new List<Byte>(), null);
			
			Message.messages["Client_onHelloCB"] = new Message(521, "Client_onHelloCB", -1, -1, new List<Byte>(), 
				KBEngineApp.app.GetType().GetMethod("Client_onHelloCB"));
			Message.clientMessages[Message.messages["Client_onHelloCB"].id] = Message.messages["Client_onHelloCB"];
			
			Message.messages["Client_onScriptVersionNotMatch"] = new Message(522, "Client_onScriptVersionNotMatch", -1, -1, new List<Byte>(), 
				KBEngineApp.app.GetType().GetMethod("Client_onScriptVersionNotMatch"));
			Message.clientMessages[Message.messages["Client_onScriptVersionNotMatch"].id] = Message.messages["Client_onScriptVersionNotMatch"];

			Message.messages["Client_onVersionNotMatch"] = new Message(523, "Client_onVersionNotMatch", -1, -1, new List<Byte>(), 
				KBEngineApp.app.GetType().GetMethod("Client_onVersionNotMatch"));
			Message.clientMessages[Message.messages["Client_onVersionNotMatch"].id] = Message.messages["Client_onVersionNotMatch"];
			
			Message.messages["Client_onImportClientMessages"] = new Message(518, "Client_onImportClientMessages", -1, -1, new List<Byte>(), 
				KBEngineApp.app.GetType().GetMethod("Client_onImportClientMessages"));
			Message.clientMessages[Message.messages["Client_onImportClientMessages"].id] = Message.messages["Client_onImportClientMessages"];
		}
		
		public Message(MessageID msgid, string msgname, Int16 length, sbyte argstype, List<Byte> msgargtypes, System.Reflection.MethodInfo msghandler)
		{
			id = msgid;
			name = msgname;
			msglen = length;
			handler = msghandler;
			argsType = argstype;
			
			// 对该消息的所有参数绑定反序列化方法，改方法能够将二进制流转化为参数需要的值
			// 在服务端下发消息数据时会用到
			argtypes = new KBEDATATYPE_BASE[msgargtypes.Count];
			for(int i=0; i<msgargtypes.Count; i++)
			{
				if(!EntityDef.id2datatypes.TryGetValue(msgargtypes[i], out argtypes[i]))
				{
					Dbg.ERROR_MSG("Message::Message(): argtype(" + msgargtypes[i] + ") is not found!");
				}
			}
			
			// Dbg.DEBUG_MSG(string.Format("Message::Message(): ({0}/{1}/{2})!", 
			//	msgname, msgid, msglen));
		}
		
		/*
			从二进制数据流中创建该消息的参数数据
		*/
		public object[] createFromStream(MemoryStream msgstream)
		{
			if(argtypes.Length <= 0)
				return new object[]{msgstream};
			
			object[] result = new object[argtypes.Length];
			
			for(int i=0; i<argtypes.Length; i++)
			{
				result[i] = argtypes[i].createFromStream(msgstream);
			}
			
			return result;
		}
		
		/*
			将一个消息包反序列化后交给消息相关联的函数处理
			例如：KBEngineApp.Client_onRemoteMethodCall
		*/
		public void handleMessage(MemoryStream msgstream)
		{
			if(argtypes.Length <= 0)
			{
				if(argsType < 0)
					handler.Invoke(KBEngineApp.app, new object[]{msgstream});
				else
					handler.Invoke(KBEngineApp.app, new object[]{});
			}
			else
			{
				handler.Invoke(KBEngineApp.app, createFromStream(msgstream));
			}
		}
    }
} 
