namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	
	using MessageID = System.UInt16;
	using MessageLength = System.UInt16;
	
    public class MessageReader
    {
		enum READ_STATE
		{
			READ_STATE_MSGID = 0,
			READ_STATE_MSGLEN = 1,
			READ_STATE_BODY = 2
		}
		
		private MessageID msgid = 0;
		private MessageLength msglen = 0;
		private MessageLength expectSize = 2;
		private READ_STATE state = READ_STATE.READ_STATE_MSGID;
		private MemoryStream stream = new MemoryStream();
		
		public MessageReader()
		{
		}
		
		public void process(byte[] datas, MessageLength length)
		{
			MessageLength totallen = 0;
			
			while(length > 0 && expectSize > 0)
			{
				if(state == READ_STATE.READ_STATE_MSGID)
				{
					if(length >= expectSize)
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, expectSize);
						totallen += expectSize;
						stream.wpos += expectSize;
						length -= expectSize;
						
						msgid = stream.readUint16();
						stream.clear();

						Message msg = Message.clientMessages[msgid];

						if(msg.msglen == -1)
						{
							state = READ_STATE.READ_STATE_MSGLEN;
							expectSize = 2;
						}
						else
						{
							state = READ_STATE.READ_STATE_BODY;
							expectSize = (MessageLength)msg.msglen;
						}
					}
					else
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, length);
						stream.wpos += length;
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
						stream.wpos += expectSize;
						length -= expectSize;
						
						msglen = stream.readUint16();
						stream.clear();

						state = READ_STATE.READ_STATE_BODY;
						expectSize = msglen;
					}
					else
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, length);
						stream.wpos += length;
						expectSize -= length;
						break;
					}
				}
				else if(state == READ_STATE.READ_STATE_BODY)
				{
					if(length >= expectSize)
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, expectSize);
						totallen += expectSize;
						stream.wpos += expectSize;
						length -= expectSize;

						Message msg = Message.clientMessages[msgid];

						msg.handleMessage(stream);
						stream.clear();
						
						state = READ_STATE.READ_STATE_MSGID;
						expectSize = 2;
					}
					else
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, length);
						stream.wpos += length;
						expectSize -= length;
						break;
					}
				}
			}
		}
    }
} 
