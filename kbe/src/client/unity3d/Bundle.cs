namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections;
	using System.Collections.Generic;
	
    public class Bundle 
    {
    	public MemoryStream stream = new MemoryStream();
		public List<MemoryStream> streamList = new List<MemoryStream>();
		public int numMessage = 0;
		public int messageLength = 0;
		public Message msgtype = null;
		
		public Bundle()
		{
		}
		
		public void newMessage(Message mt)
		{
			fini(false);
			
			msgtype = mt;
			numMessage += 1;

			writeUint16(msgtype.id);

			if(msgtype.msglen == -1)
			{
				writeUint16(0);
				messageLength = 0;
			}
		}
		
		public void writeMsgLength()
		{
			if(msgtype.msglen != -1)
				return;
			
			if(stream.opsize() >= messageLength)
			{
				int idx = (int)stream.opsize() - messageLength - 2;
				stream.data()[idx] = (Byte)(messageLength & 0xff);
				stream.data()[idx + 1] = (Byte)(messageLength >> 8 & 0xff);
			}
			else
			{
				int size = messageLength - (int)stream.opsize();
				byte[] data = streamList[numMessage - 1].data();
				
				int idx = data.Length - size - 2;
				
				data[idx] = (Byte)(messageLength & 0xff);
				data[idx + 1] = (Byte)(messageLength >> 8 & 0xff);
			}
		}
		
		public void fini(bool issend)
		{
			if(numMessage > 0)
			{
				writeMsgLength();
				if(stream != null)
					streamList.Add(stream);
			}
			
			if(issend)
			{
				numMessage = 0;
				msgtype = null;
			}
		}
		
		public void send(NetworkInterface networkInterface)
		{
			fini(true);
			
			for(int i=0; i<streamList.Count; i++)
			{
				stream = streamList[i];
				networkInterface.send(stream.getbuffer());
			}
			
			streamList.Clear();
			stream = new MemoryStream();
		}
		
		public void checkStream(int v)
		{
			if(v > stream.fillfree())
			{
				streamList.Add(stream);
				stream = new MemoryStream();
			}
	
			messageLength += v;
		}
		
		//---------------------------------------------------------------------------------
		public void writeInt8(SByte v)
		{
			checkStream(1);
			stream.writeInt8(v);
		}
	
		public void writeInt16(Int16 v)
		{
			checkStream(2);
			stream.writeInt16(v);
		}
			
		public void writeInt32(Int32 v)
		{
			checkStream(4);
			stream.writeInt32(v);
		}
	
		public void writeInt64(Int64 v)
		{
			checkStream(8);
			stream.writeInt64(v);
		}
		
		public void writeUint8(Byte v)
		{
			checkStream(1);
			stream.writeUint8(v);
		}
	
		public void writeUint16(UInt16 v)
		{
			checkStream(2);
			stream.writeUint16(v);
		}
			
		public void writeUint32(UInt32 v)
		{
			checkStream(4);
			stream.writeUint32(v);
		}
	
		public void writeUint64(UInt64 v)
		{
			checkStream(8);
			stream.writeUint64(v);
		}
		
		public void writeFloat(float v)
		{
			checkStream(4);
			stream.writeFloat(v);
		}
	
		public void writeDouble(double v)
		{
			checkStream(8);
			stream.writeDouble(v);
		}
		
		public void writeString(string v)
		{
			checkStream(v.Length + 1);
			stream.writeString(v);
		}
		
		public void writeBlob(byte[] v)
		{
			checkStream(v.Length + 4);
			stream.writeBlob(v);
		}
    }
} 
