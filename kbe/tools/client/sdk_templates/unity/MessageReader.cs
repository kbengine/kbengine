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
    public class MessageReader
    {
		enum READ_STATE
		{
			// 消息ID
			READ_STATE_MSGID = 0,

			// 消息的长度65535以内
			READ_STATE_MSGLEN = 1,

			// 当上面的消息长度都无法到达要求时使用扩展长度
			// uint32
			READ_STATE_MSGLEN_EX = 2,

			// 消息的内容
			READ_STATE_BODY = 3
		}
		
		private MessageID msgid = 0;
		private MessageLength msglen = 0;
		private MessageLengthEx expectSize = 2;
		private READ_STATE state = READ_STATE.READ_STATE_MSGID;
		private MemoryStream stream = new MemoryStream();
		
		public MessageReader()
		{
		}
		
		public void process(byte[] datas, MessageLengthEx offset, MessageLengthEx length)
		{
			MessageLengthEx totallen = offset;
			
			while(length > 0 && expectSize > 0)
			{
				if(state == READ_STATE.READ_STATE_MSGID)
				{
					if(length >= expectSize)
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, expectSize);
						totallen += expectSize;
						stream.wpos += (int)expectSize;
						length -= expectSize;
						msgid = stream.readUint16();
						stream.clear();

						Message msg = Message.clientMessages[msgid];

						if(msg.msglen == -1)
						{
							state = READ_STATE.READ_STATE_MSGLEN;
							expectSize = 2;
						}
						else if(msg.msglen == 0)
						{
							// 如果是0个参数的消息，那么没有后续内容可读了，处理本条消息并且直接跳到下一条消息
							#if UNITY_EDITOR
							Dbg.profileStart(msg.name);
							#endif

							msg.handleMessage(stream);

							#if UNITY_EDITOR
							Dbg.profileEnd(msg.name);
							#endif

							state = READ_STATE.READ_STATE_MSGID;
							expectSize = 2;
						}
						else
						{
							expectSize = (MessageLengthEx)msg.msglen;
							state = READ_STATE.READ_STATE_BODY;
						}
					}
					else
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, length);
						stream.wpos += (int)length;
						expectSize -= length;
						break;
					}
				}
				else if(state == READ_STATE.READ_STATE_MSGLEN)
				{
					if(length >= expectSize)
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, expectSize);
						totallen += expectSize;
						stream.wpos += (int)expectSize;
						length -= expectSize;
						
						msglen = stream.readUint16();
						stream.clear();
						
						// 长度扩展
						if(msglen >= 65535)
						{
							state = READ_STATE.READ_STATE_MSGLEN_EX;
							expectSize = 4;
						}
						else
						{
							state = READ_STATE.READ_STATE_BODY;
							expectSize = msglen;
						}
					}
					else
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, length);
						stream.wpos += (int)length;
						expectSize -= length;
						break;
					}
				}
				else if(state == READ_STATE.READ_STATE_MSGLEN_EX)
				{
					if(length >= expectSize)
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, expectSize);
						totallen += expectSize;
						stream.wpos += (int)expectSize;
						length -= expectSize;
						
						expectSize = stream.readUint32();
						stream.clear();
						
						state = READ_STATE.READ_STATE_BODY;
					}
					else
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, length);
						stream.wpos += (int)length;
						expectSize -= length;
						break;
					}
				}
				else if(state == READ_STATE.READ_STATE_BODY)
				{
					if(length >= expectSize)
					{
						stream.append (datas, totallen, expectSize);
						totallen += expectSize;
						length -= expectSize;

						Message msg = Message.clientMessages[msgid];
						
#if UNITY_EDITOR
						Dbg.profileStart(msg.name);
#endif

						msg.handleMessage(stream);
#if UNITY_EDITOR
						Dbg.profileEnd(msg.name);
#endif
						
						stream.clear();
						
						state = READ_STATE.READ_STATE_MSGID;
						expectSize = 2;
					}
					else
					{
						stream.append (datas, totallen, length);
						expectSize -= length;
						break;
					}
				}
			}
		}
    }
} 
