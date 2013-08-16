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
						Array.Copy(datas, 0, stream.data(), stream.wpos, expectSize);
						length -= expectSize;
						expectSize = 0;
						
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
						
						totallen = 2;
						Debug.Log(string.Format("MessageReader::process(): msgid={0}/{1}/{2}, msgname={3}!", 
							msg.id, msg.msglen, expectSize, msg.name));
					}
					else
					{
						expectSize -= length;
						Array.Copy(datas, 0, stream.data(), stream.wpos, length);
						length = 0;
						break;
					}
				}
				else if(state == READ_STATE.READ_STATE_MSGLEN)
				{
					if(length >= expectSize)
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, expectSize);
						length -= expectSize;
						
						msglen = stream.readUint16();
						stream.clear();
						
						Debug.Log(string.Format("MessageReader::process(): msglen={0}!", msglen));
						
						state = READ_STATE.READ_STATE_BODY;
						expectSize = msglen;
						totallen = 4;
					}
					else
					{
						expectSize -= length;
						Array.Copy(datas, totallen, stream.data(), stream.wpos, length);
						length = 0;
						break;
					}
				}
				else if(state == READ_STATE.READ_STATE_BODY)
				{
					if(length >= expectSize)
					{
						Array.Copy(datas, totallen, stream.data(), stream.wpos, expectSize);
						length -= expectSize;
						totallen += expectSize;
						Message msg = Message.clientMessages[msgid];	
						stream.wpos += expectSize;
						Debug.Log(string.Format("MessageReader::process(): handleMessage={0}, msglen={1}!", msg.name, stream.opsize()));
						Debug.Log(string.Format("MessageReader::process(): rpos({0}), wpos({1}), msg={2}!", stream.rpos, stream.wpos, stream.toString()));
						msg.handleMessage(stream);
						stream.clear();
						
						state = READ_STATE.READ_STATE_MSGID;
						expectSize = 2;
					}
					else
					{
						expectSize -= length;
						totallen += length;
						Array.Copy(datas, totallen, stream.data(), stream.wpos, length);
						stream.wpos += length;
						length = 0;
						break;
					}
				}
			}
		}
    }
    
} 
