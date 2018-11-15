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
		由于KCP是模拟的TCP流，一个UDP包中可能包含一个或者多个包，并且最后一个包可能是不完整的，因此直接继承MessageReaderTCP
	*/
    public class MessageReaderKCP : MessageReaderTCP
    {
    }
} 
