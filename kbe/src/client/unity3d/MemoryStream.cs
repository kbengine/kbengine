namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Net; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Text;
    using System.Threading; 
	
    public class MemoryStream 
    {
    	public const int BUFFER_MAX = 1460 * 4;
    	
    	public int rpos = 0;
    	public int wpos = 0;
    	private byte[] datas_ = new byte[BUFFER_MAX]; 
    	
    	public byte[] data()
    	{
    		return datas_;
    	}
		
		public void setData(byte[] data)
		{
			datas_ = data;
		}
		
		//---------------------------------------------------------------------------------
		public SByte readInt8()
		{
			return (SByte)datas_[rpos++];
		}
	
		public Int16 readInt16()
		{
			rpos += 2;
			return BitConverter.ToInt16(datas_, rpos - 2);
		}
			
		public Int32 readInt32()
		{
			rpos += 4;
			return BitConverter.ToInt32(datas_, rpos - 4);
		}
	
		public Int64 readInt64()
		{
			rpos += 8;
			return BitConverter.ToInt64(datas_, rpos - 8);
		}
		
		public Byte readUint8()
		{
			return datas_[rpos++];
		}
	
		public UInt16 readUint16()
		{
			rpos += 2;
			return BitConverter.ToUInt16(datas_, rpos - 2);
		}

		public UInt32 readUint32()
		{
			rpos += 4;
			return BitConverter.ToUInt32(datas_, rpos - 4);
		}
		
		public UInt64 readUint64()
		{
			rpos += 8;
			return BitConverter.ToUInt64(datas_, rpos - 8);
		}
		
		public float readFloat()
		{
			rpos += 4;
			return BitConverter.ToSingle(datas_, rpos - 4);
		}

		public double readDouble()
		{
			rpos += 8;
			return BitConverter.ToDouble(datas_, rpos - 8);
		}
		
		public string readString()
		{
			string s = "";
			
			while(true)
			{
				byte v = datas_[rpos++];
				if(v == 0)
				{
					break;
				}
				
				char c = System.Convert.ToChar(v);
				s += c;
			}
			
			return s;
		}
	
		public byte[] readBlob()
		{
			UInt32 size = readUint32();
			byte[] buf = new byte[size];
			
			Array.Copy(datas_, rpos, buf, 0, size);
			rpos += (int)size;
			return buf;
		}
	
		public Vector2 readPackXZ()
		{
			Byte v1 = readUint8();
			Byte v2 = readUint8();
			Byte v3 = readUint8();
			
			Vector2 vec = new Vector2(0, 0);
			return vec;
		}
	
		public float readPackY()
		{
			UInt16 v = readUint16();
			return 0.0f;
		}
		
		//---------------------------------------------------------------------------------
		public void writeInt8(SByte v)
		{
			datas_[wpos++] = (Byte)v;
		}
	
		public void writeInt16(Int16 v)
		{	
			writeInt8((SByte)(v & 0xff));
			writeInt8((SByte)(v >> 8 & 0xff));
		}
			
		public void writeInt32(Int32 v)
		{
			for(int i=0; i<4; i++)
				writeInt8((SByte)(v >> i * 8 & 0xff));
		}
	
		public void writeInt64(Int64 v)
		{
			byte[] getdata = BitConverter.GetBytes(v);
			for(int i=0; i<getdata.Length; i++)
			{
				datas_[wpos++] = getdata[i];
			}
		}
		
		public void writeUint8(Byte v)
		{
			datas_[wpos++] = v;
		}
	
		public void writeUint16(UInt16 v)
		{
			writeUint8((Byte)(v & 0xff));
			writeUint8((Byte)(v >> 8 & 0xff));
		}
			
		public void writeUint32(UInt32 v)
		{
			for(int i=0; i<4; i++)
				writeUint8((Byte)(v >> i * 8 & 0xff));
		}
	
		public void writeUint64(UInt64 v)
		{
			byte[] getdata = BitConverter.GetBytes(v);
			for(int i=0; i<getdata.Length; i++)
			{
				datas_[wpos++] = getdata[i];
			}
		}
		
		public void writeFloat(float v)
		{
			byte[] getdata = BitConverter.GetBytes(v);
			for(int i=0; i<getdata.Length; i++)
			{
				datas_[wpos++] = getdata[i];
			}
		}
	
		public void writeDouble(double v)
		{
			byte[] getdata = BitConverter.GetBytes(v);
			for(int i=0; i<getdata.Length; i++)
			{
				datas_[wpos++] = getdata[i];
			}
		}
	
		public void writeBlob(byte[] v)
		{
			UInt32 size = (UInt32)v.Length;
			if(size + 4> fillfree())
			{
				Debug.LogError("memorystream::writeBlob: no free!");
				return;
			}
			
			writeUint32(size);
		
			for(UInt32 i=0; i<size; i++)
			{
				datas_[wpos++] = v[i];
			}
			
			this.wpos += v.Length;
		}
		
		public void writeString(string v)
		{
			if(v.Length > fillfree())
			{
				Debug.LogError("memorystream::writeString: no free!");
				return;
			}

			byte[] getdata = System.Text.Encoding.ASCII.GetBytes(v);
			for(int i=0; i<getdata.Length; i++)
			{
				datas_[wpos++] = getdata[i];
			}
			
			datas_[wpos++] = 0;
		}
		
		//---------------------------------------------------------------------------------
		public void readSkip(UInt32 v)
		{
			rpos += (int)v;
		}
		
		//---------------------------------------------------------------------------------
		public UInt32 fillfree()
		{
			return (UInt32)(BUFFER_MAX - wpos);
		}
	
		//---------------------------------------------------------------------------------
		public UInt32 opsize()
		{
			return (UInt32)(wpos - rpos);
		}
	
		//---------------------------------------------------------------------------------
		public bool readEOF()
		{
			return (BUFFER_MAX - rpos) <= 0;
		}
		
		//---------------------------------------------------------------------------------
		public UInt32 totalsize()
		{
			return opsize();
		}
	
		//---------------------------------------------------------------------------------
		public void opfini()
		{
			rpos = wpos;
		}
		
		//---------------------------------------------------------------------------------
		public void clear()
		{
			rpos = wpos = 0;
		}
		
		//---------------------------------------------------------------------------------
		public byte[] getbuffer()
		{
			byte[] buf = new byte[opsize()];
			Array.Copy(data(), rpos, buf, 0, opsize());
			return buf;
		}
		
		//---------------------------------------------------------------------------------
		public string toString()
		{
			string s = "";
			byte[] buf = getbuffer();
			for(int i=0; i<buf.Length; i++)
			{
				s += buf[i];
				s += " ";
			}
			
			return s;
		}
    }
    
} 
