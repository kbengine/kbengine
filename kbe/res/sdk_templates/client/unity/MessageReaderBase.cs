namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	
	using MessageID = System.UInt16;
	using MessageLength = System.UInt16;
	using MessageLengthEx = System.UInt32;
	
	/*
		消息阅读模块
		从数据包流中分析出所有的消息包并将其交给对应的消息处理函数
	*/
    public abstract class MessageReaderBase
    {
			public abstract void process(byte[] datas, MessageLengthEx offset, MessageLengthEx length);
    }
} 
