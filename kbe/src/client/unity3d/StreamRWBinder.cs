namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
    public class StreamRWBinder 
    {
		public static System.Reflection.MethodInfo bindwriter(Byte argType)
		{
			Type writer = typeof(Bundle);
			System.Reflection.MethodInfo info = null;
			
			if(argType == EntityDef.datatype2id["UINT8"])
			{
				info = writer.GetMethod("writeUint8");
			}
			else if(argType == EntityDef.datatype2id["UINT16"])
			{
				info = writer.GetMethod("writeUint16");
			}
			else if(argType == EntityDef.datatype2id["UINT32"])
			{
				info = writer.GetMethod("writeUint32");
			}
			else if(argType == EntityDef.datatype2id["UINT64"])
			{
				info = writer.GetMethod("writeUint64");
			}
			else if(argType == EntityDef.datatype2id["INT8"])
			{
				info = writer.GetMethod("writeInt8");
			}
			else if(argType == EntityDef.datatype2id["INT16"])
			{
				info = writer.GetMethod("writeInt16");
			}
			else if(argType == EntityDef.datatype2id["INT32"])
			{
				info = writer.GetMethod("writeInt32");
			}
			else if(argType == EntityDef.datatype2id["INT64"])
			{
				info = writer.GetMethod("writeInt64");
			}
			else if(argType == EntityDef.datatype2id["FLOAT"])
			{
				info = writer.GetMethod("writeFloat");
			}
			else if(argType == EntityDef.datatype2id["DOUBLE"])
			{
				info = writer.GetMethod("writeDouble");
			}
			else if(argType == EntityDef.datatype2id["STRING"])
			{
				info = writer.GetMethod("writeString");
			}
			else if(argType == EntityDef.datatype2id["FIXED_DICT"])
			{
				info = writer.GetMethod("writeStream");
			}
			else if(argType == EntityDef.datatype2id["ARRAY"])
			{
				info = writer.GetMethod("writeStream");
			}
			else
			{
				info = writer.GetMethod("writeStream");
			}
			
			if(info == null)
				Dbg.WARNING_MSG("StreamRWBinder:: bindwriter(" + argType + ") is error!");
			
			return info;
		}
		
		public static System.Reflection.MethodInfo bindReader(Byte argType)
		{
			Type reader = typeof(MemoryStream);
			System.Reflection.MethodInfo info = null;
			
			if(argType == EntityDef.datatype2id["UINT8"])
			{
				info =  reader.GetMethod("readUint8");
			}
			else if(argType == EntityDef.datatype2id["UINT16"])
			{
				info =  reader.GetMethod("readUint16");
			}
			else if(argType == EntityDef.datatype2id["UINT32"])
			{
				info =  reader.GetMethod("readUint32");
			}
			else if(argType == EntityDef.datatype2id["UINT64"])
			{
				info =  reader.GetMethod("readUint64");
			}
			else if(argType == EntityDef.datatype2id["INT8"])
			{
				info =  reader.GetMethod("readInt8");
			}
			else if(argType == EntityDef.datatype2id["INT16"])
			{
				info =  reader.GetMethod("readInt16");
			}
			else if(argType == EntityDef.datatype2id["INT32"])
			{
				info =  reader.GetMethod("readInt32");
			}
			else if(argType == EntityDef.datatype2id["INT64"])
			{
				info =  reader.GetMethod("readInt64");
			}
			else if(argType == EntityDef.datatype2id["FLOAT"])
			{
				info =  reader.GetMethod("readFloat");
			}
			else if(argType == EntityDef.datatype2id["DOUBLE"])
			{
				info =  reader.GetMethod("readDouble");
			}
			else if(argType == EntityDef.datatype2id["STRING"])
			{
				info =  reader.GetMethod("readString");
			}
			else if(argType == EntityDef.datatype2id["PYTHON"])
			{
				info =  reader.GetMethod("readStream");
			}
			else if(argType == EntityDef.datatype2id["VECTOR2"])
			{
				info =  reader.GetMethod("readStream");
			}
			else if(argType == EntityDef.datatype2id["VECTOR3"])
			{
				info =  reader.GetMethod("readStream");
			}
			else if(argType == EntityDef.datatype2id["VECTOR4"])
			{
				info =  reader.GetMethod("readStream");
			}
			else if(argType == EntityDef.datatype2id["BLOB"])
			{
				info =  reader.GetMethod("readStream");
			}
			else if(argType == EntityDef.datatype2id["UNICODE"])
			{
				info =  reader.GetMethod("readStream");
			}
			else if(argType == EntityDef.datatype2id["FIXED_DICT"])
			{
				info =  reader.GetMethod("readStream");
			}
			else if(argType == EntityDef.datatype2id["ARRAY"])
			{
				info =  reader.GetMethod("readStream");
			}
			else
			{
				info =  reader.GetMethod("readStream");
			}
			
			if(info == null)
				Dbg.WARNING_MSG("StreamRWBinder:: bindReader(" + argType + ") is error!");
			
			return info;
		}
    }
    
} 
