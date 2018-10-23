namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
	using MessageID = System.UInt16;
	
	/*
		entitydef所支持的基本数据类型
		改模块中的类抽象出了所有的支持类型并提供了这些类型的数据序列化成二进制数据与反序列化操作（主要用于网络通讯的打包与解包）
	*/
	public class DATATYPE_BASE
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
		
		public virtual bool isSameType(object v)
		{
			return v == null;
		}
	}
	
	public class DATATYPE_INT8 : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if (!KBEMath.isNumeric (v))
				return false;

			decimal v1 = Convert.ToDecimal (v);
			return v1 >= sbyte.MinValue && v1 <= sbyte.MaxValue;
		}
	}
	
	public class DATATYPE_INT16 : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if (!KBEMath.isNumeric (v))
				return false;

			decimal v1 = Convert.ToDecimal (v);
			return v1 >= Int16.MinValue && v1 <= Int16.MaxValue;
		}
	}
	
	public class DATATYPE_INT32 : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if (!KBEMath.isNumeric (v))
				return false;

			decimal v1 = Convert.ToDecimal (v);
			return v1 >= Int32.MinValue && v1 <= Int32.MaxValue;
		}
	}
	
	public class DATATYPE_INT64 : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if (!KBEMath.isNumeric (v))
				return false;

			decimal v1 = Convert.ToDecimal (v);
			return v1 >= Int64.MinValue && v1 <= Int64.MaxValue;
		}
	}
	
	public class DATATYPE_UINT8 : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if (!KBEMath.isNumeric (v))
				return false;

			decimal v1 = Convert.ToDecimal (v);
			return v1 >= Byte.MinValue && v1 <= Byte.MaxValue;
		}
	}
	
	public class DATATYPE_UINT16 : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if (!KBEMath.isNumeric (v))
				return false;

			decimal v1 = Convert.ToDecimal (v);
			return v1 >= UInt16.MinValue && v1 <= UInt16.MaxValue;
		}
	}
	
	public class DATATYPE_UINT32 : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if (!KBEMath.isNumeric (v))
				return false;

			decimal v1 = Convert.ToDecimal (v);
			return v1 >= UInt32.MinValue && v1 <= UInt32.MaxValue;
		}
	}
	
	public class DATATYPE_UINT64 : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if (!KBEMath.isNumeric (v))
				return false;

			decimal v1 = Convert.ToDecimal (v);
			return v1 >= UInt64.MinValue && v1 <= UInt64.MaxValue;
		}
	}
	
	public class DATATYPE_FLOAT : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if(v is float)
				return (float)v >= float.MinValue && (float)v <= float.MaxValue;
			else if(v is double)
				return (double)v >= float.MinValue && (double)v <= float.MaxValue;
			
			return false;
		}
	}
	
	public class DATATYPE_DOUBLE : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			if(v is float)
				return (float)v >= double.MinValue && (float)v <= double.MaxValue;
			else if(v is double)
				return (double)v >= double.MinValue && (double)v <= double.MaxValue;
			
			return false;
		}
	}
	
	public class DATATYPE_STRING : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			return v != null && v.GetType() == typeof(string);
		}
	}
	
	public class DATATYPE_VECTOR2 : DATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return new Vector2(stream.readFloat(), stream.readFloat());
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeFloat(((Vector2)v).x);
			stream.writeFloat(((Vector2)v).y);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new Vector2(0.0f, 0.0f);
		}
		
		public override bool isSameType(object v)
		{
			return v != null && v.GetType() == typeof(Vector2);
		}
	}
	
	public class DATATYPE_VECTOR3 : DATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return new Vector3(stream.readFloat(), stream.readFloat(), stream.readFloat());
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeFloat(((Vector3)v).x);
			stream.writeFloat(((Vector3)v).y);
			stream.writeFloat(((Vector3)v).z);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new Vector3(0.0f, 0.0f, 0.0f);
		}
		
		public override bool isSameType(object v)
		{
			return v != null && v.GetType() == typeof(Vector3);
		}
	}
	
	public class DATATYPE_VECTOR4 : DATATYPE_BASE
	{
		public override object createFromStream(MemoryStream stream)
		{
			return new Vector4(stream.readFloat(), stream.readFloat(), stream.readFloat(), stream.readFloat());
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeFloat(((Vector4)v).x);
			stream.writeFloat(((Vector4)v).y);
			stream.writeFloat(((Vector4)v).z);
			stream.writeFloat(((Vector4)v).w);
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		
		public override bool isSameType(object v)
		{
			return v != null && v.GetType() == typeof(Vector4);
		}
	}
	
	public class DATATYPE_PYTHON : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			return v != null && v.GetType() == typeof(byte[]);
		}
	}
	
	public class DATATYPE_UNICODE : DATATYPE_BASE
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
			return v;
		}
		
		public override bool isSameType(object v)
		{
			return v != null && v.GetType() == typeof(string);
		}
	}
	
	public class DATATYPE_ENTITYCALL : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			return v != null && v.GetType() == typeof(byte[]);
		}
	}
	
	public class DATATYPE_BLOB : DATATYPE_BASE
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
		
		public override bool isSameType(object v)
		{
			return v != null && v.GetType() == typeof(byte[]);
		}
	}
	
	public class DATATYPE_ARRAY : DATATYPE_BASE
	{
		public object vtype;
		
		public override void bind()
		{
			if(vtype.GetType ().BaseType.ToString() == "KBEngine.DATATYPE_BASE")
				((DATATYPE_BASE)vtype).bind();
			else
				if(EntityDef.id2datatypes.ContainsKey((UInt16)vtype))
					vtype = EntityDef.id2datatypes[(UInt16)vtype];
		}
		
		public override object createFromStream(MemoryStream stream)
		{
			UInt32 size = stream.readUint32();
			List<object> datas = new List<object>();
			
			while(size > 0)
			{
				size--;
		//		datas.Add(((DATATYPE_BASE)vtype).createFromStream(stream));
			};
			
			return datas;
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			stream.writeUint32((UInt32)((List<object>)v).Count);
			for(int i=0; i<((List<object>)v).Count; i++)
			{
		//		((DATATYPE_BASE)vtype).addToStream(stream, ((List<object>)v)[i]);
			}
		}
		
		public override object parseDefaultValStr(string v)
		{
			return new byte[0];
		}
		
		public override bool isSameType(object v)
		{
			if(vtype.GetType ().BaseType.ToString() != "KBEngine.DATATYPE_BASE")
			{
				Dbg.ERROR_MSG(string.Format("DATATYPE_ARRAY::isSameType: has not bind! baseType={0}", 
					vtype.GetType ().BaseType.ToString()));
				return false;
			}

			if(v == null || v.GetType() != typeof(List<object>))
			{
				return false;
			}
			
			for(int i=0; i<((List<object>)v).Count; i++)
			{
				if(!((DATATYPE_BASE)vtype).isSameType(((List<object>)v)[i]))
				{
					return false;
				}
			}
			
			return true;
		}
	}
	
	public class DATATYPE_FIXED_DICT : DATATYPE_BASE
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
				
				if(type.GetType ().BaseType.ToString() == "KBEngine.DATATYPE_BASE")
					((DATATYPE_BASE)type).bind();
				else
					if(EntityDef.id2datatypes.ContainsKey((UInt16)type))
						dicttype[itemkey] = EntityDef.id2datatypes[(UInt16)type];
			}
		}
		
		public override object createFromStream(MemoryStream stream)
		{
			Dictionary<string, object> datas = new Dictionary<string, object>();
			foreach(string itemkey in dicttype.Keys)
			{
			//	datas[itemkey] = ((DATATYPE_BASE)dicttype[itemkey]).createFromStream(stream);
			}
			
			return datas;
		}
		
		public override void addToStream(Bundle stream, object v)
		{
			foreach(string itemkey in dicttype.Keys)
			{
		//		((DATATYPE_BASE)dicttype[itemkey]).addToStream(stream, ((Dictionary<string, object>)v)[itemkey]);
			}
		}
		
		public override object parseDefaultValStr(string v)
		{
			Dictionary<string, object> datas = new Dictionary<string, object>();
			foreach(string itemkey in dicttype.Keys)
			{
				datas[itemkey] = ((DATATYPE_BASE)dicttype[itemkey]).parseDefaultValStr("");
			}
			
			return datas;
		}
		
		public override bool isSameType(object v)
		{
			if(v == null || v.GetType() != typeof(Dictionary<string, object>))
				return false;
						
			foreach(KeyValuePair<string, object> item in dicttype)
			{
				object value;
				if(((Dictionary<string, object>)v).TryGetValue(item.Key, out value))
				{
					if(!((DATATYPE_BASE)item.Value).isSameType(value))
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			
			return true;
		}
	}
} 
