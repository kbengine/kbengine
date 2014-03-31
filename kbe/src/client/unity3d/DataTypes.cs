namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
	using MessageID = System.UInt16;
	
	public class KBEDATATYPE_BASE
	{
		public virtual void bind()
		{
		}
		
		public virtual object createFromStream(MemoryStream stream)
		{
			return null;
		}
		
		public virtual void addToStream(Bundle stream, object v)
		{
		}
		
		public virtual object parseDefaultValStr(string v)
		{
			return null;
		}
	}
	
	public class KBEDATATYPE_INT8 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readInt8();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeInt8(Convert.ToSByte(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			SByte ret = 0;
			SByte.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_INT16 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readInt16();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeInt16(Convert.ToInt16(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			Int16 ret = 0;
			Int16.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_INT32 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readInt32();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeInt32(Convert.ToInt32(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			Int32 ret = 0;
			Int32.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_INT64 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readInt64();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeInt64(Convert.ToInt64(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			Int64 ret = 0;
			Int64.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_UINT8 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readUint8();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint8(Convert.ToByte(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			Byte ret = 0;
			Byte.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_UINT16 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readUint16();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint16(Convert.ToUInt16(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			UInt16 ret = 0;
			UInt16.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_UINT32 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readUint32();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint32(Convert.ToUInt32(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			UInt32 ret = 0;
			UInt32.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_UINT64 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readUint64();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint64(Convert.ToUInt64(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			UInt64 ret = 0;
			UInt64.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_FLOAT : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readFloat();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeFloat((float)Convert.ToDouble(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			float ret = 0.0f;
			float.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_DOUBLE : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readDouble();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeDouble(Convert.ToDouble(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			double ret = 0.0;
			double.TryParse(v, out ret);
			return ret;
		}
	}
	
	public class KBEDATATYPE_STRING : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readString();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeString(Convert.ToString(v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			return v;
		}
	}
	
	public class KBEDATATYPE_VECTOR2 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			UInt32 size = stream.readUint32();
			if(2 != size)
			{
				Dbg.ERROR_MSG(string.Format("KBEDATATYPE_VECTOR2::createFromStream: size({0}) is error!", size));
			}
			
			return new Vector2(stream.readFloat(), stream.readFloat());
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint32(2);
			stream.writeFloat(((Vector2)v).x);
			stream.writeFloat(((Vector2)v).y);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new Vector2(0.0f, 0.0f);
		}
	}
	
	public class KBEDATATYPE_VECTOR3 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			UInt32 size = stream.readUint32();
			if(3 != size)
			{
				Dbg.ERROR_MSG(string.Format("KBEDATATYPE_VECTOR3::createFromStream: size({0}) is error!", size));
			}
			
			return new Vector3(stream.readFloat(), stream.readFloat(), stream.readFloat());
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint32(3);
			stream.writeFloat(((Vector3)v).x);
			stream.writeFloat(((Vector3)v).y);
			stream.writeFloat(((Vector3)v).z);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new Vector3(0.0f, 0.0f, 0.0f);
		}
	}
	
	public class KBEDATATYPE_VECTOR4 : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			UInt32 size = stream.readUint32();
			if(4 != size)
			{
				Dbg.ERROR_MSG(string.Format("KBEDATATYPE_VECTOR4::createFromStream: size({0}) is error!", size));
			}
			
			return new Vector4(stream.readFloat(), stream.readFloat(), stream.readFloat(), stream.readFloat());
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint32(4);
			stream.writeFloat(((Vector4)v).x);
			stream.writeFloat(((Vector4)v).y);
			stream.writeFloat(((Vector4)v).z);
			stream.writeFloat(((Vector4)v).w);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	
	public class KBEDATATYPE_PYTHON : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readBlob();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeBlob((byte[])v);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new byte[0];
		}
	}
	
	public class KBEDATATYPE_UNICODE : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return System.Text.Encoding.UTF8.GetString(stream.readBlob());
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeBlob(System.Text.Encoding.UTF8.GetBytes((string)v));
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new byte[0];
		}
	}
	
	public class KBEDATATYPE_MAILBOX : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readBlob();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeBlob((byte[])v);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new byte[0];
		}
	}
	
	public class KBEDATATYPE_BLOB : KBEDATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return stream.readBlob();
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeBlob((byte[])v);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new byte[0];
		}
	}
	
	public class KBEDATATYPE_ARRAY : KBEDATATYPE_BASE
	{
		public object type;
		
		public override void bind()
		{
			if(type.GetType() == typeof(KBEDATATYPE_BASE).GetType())
				((KBEDATATYPE_BASE)type).bind();
			else
				if(EntityDef.iddatatypes.ContainsKey((UInt16)type))
					type = EntityDef.iddatatypes[(UInt16)type];
		}
		
		public override object createFromStream(MemoryStream stream)
		{
			UInt32 size = stream.readUint32();
			List<object> datas = new List<object>();
			
			while(size > 0)
			{
				size--;
				datas.Add(((KBEDATATYPE_BASE)type).createFromStream(stream));
			};
			
			return datas;
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint32((UInt32)((List<object>)v).Count);
			for(int i=0; i<((List<object>)v).Count; i++)
			{
				((KBEDATATYPE_BASE)type).addToStream(stream, ((List<object>)v)[i]);
			}
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new byte[0];
		}
	}
	
	public class KBEDATATYPE_FIXED_DICT : KBEDATATYPE_BASE
	{
		public string implementedBy = "";
		public Dictionary<string, object> dicttype = new Dictionary<string, object>();
		
		public override void bind()
		{
			string[] keys = new string[dicttype.Keys.Count];
			dicttype.Keys.CopyTo(keys, 0);
			
			foreach(string itemkey in keys)
			{
				object type = dicttype[itemkey];
				
				if(type.GetType() == typeof(KBEDATATYPE_BASE).GetType())
					((KBEDATATYPE_BASE)type).bind();
				else
					if(EntityDef.iddatatypes.ContainsKey((UInt16)type))
						dicttype[itemkey] = EntityDef.iddatatypes[(UInt16)type];
			}
		}
		
		public override object createFromStream(MemoryStream stream)
		{
			Dictionary<string, object> datas = new Dictionary<string, object>();
			foreach(string itemkey in dicttype.Keys)
			{
				datas[itemkey] = ((KBEDATATYPE_BASE)dicttype[itemkey]).createFromStream(stream);
			}
			
			return datas;
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			foreach(string itemkey in dicttype.Keys)
			{
				((KBEDATATYPE_BASE)dicttype[itemkey]).addToStream(stream, ((Dictionary<string, object>)v)[itemkey]);
			}
		}
		
		public override object parseDefaultValStr(string v)
		{
			Dictionary<string, object> datas = new Dictionary<string, object>();
			foreach(string itemkey in dicttype.Keys)
			{
				datas[itemkey] = ((KBEDATATYPE_BASE)dicttype[itemkey]).parseDefaultValStr("");
			}
			
			return datas;
		}
	}
} 
