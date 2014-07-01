//
//  KDataTypes.h
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#ifndef __libKBEClient__KDataTypes__
#define __libKBEClient__KDataTypes__

//using namespace std;

#include <iostream>
#include <vector>
#include "KMemoryStream.h"
#include "KMessage.h"
#include "KNetworkInterface.h"
#include "KBundle.h"
#include "cocos2d.h"
#include "kazmath/kazmath.h"


typedef kmVec2 Vector2;

typedef kmVec3 Vector3;
typedef kmVec4 Vector4;

namespace KBEngineClient{

	class KMessage;
	class MemoryStream;
	class KBEDATATYPE_BASE;

	typedef	void* object; //store pointer 

	class KBEDATATYPE_BASE
	{
	public:
		uint16 did_;
		std::string name;
		//
		KBEDATATYPE_BASE(DATATYPE_UID did = 0){	did_=did;};
		KBEDATATYPE_BASE(KBEDATATYPE_BASE& other){ did_= other.did_; name = other.name; } ;
		KBEDATATYPE_BASE(KBEDATATYPE_BASE* other){ did_= other->did_; name = other->name; } ;
		virtual ~KBEDATATYPE_BASE(){};
		virtual void bind()
		{
		}
		
		virtual object createFromStream(MemoryStream& stream)
		{
			return 0;
		}
		
		virtual void addToStream(KBundle stream, object v)
		{
		}
		
		virtual object parseDefaultValStr(std::string v)
		{
			return 0;
		}
	};

	//template <typename SPECIFY_TYPE>
	//class IntType : public KBEDATATYPE_BASE
	//{
	//public:	
	//	IntType(DATATYPE_UID did = 0);
	//	virtual ~IntType();	

	//	void addToStream(MemoryStream* mstream, object v);
	//	object createFromStream(MemoryStream* mstream);
	//	object parseDefaultStr(std::string defaultVal);
	//};


	//INT8
	class KBEDATATYPE_INT8 : public KBEDATATYPE_BASE
	{
		public:
	/*		std::string name;
			KBEDATATYPE_INT8(DATATYPE_UID did ):name("int8")
			{ };
			virtual int8 createFromStream(MemoryStream* stream)
			{
				return stream->readInt8() ;
			}
		
			virtual void addToStream(KBundle stream, object v)
			{
				int8 _i = (int8) v; 
				stream.writeInt8( _i );
			}
		
			object parseDefaultValStr(std::string v)
			{
				int8 _i = (int8)atoi(v.c_str());
				return  (object) _i;
			}*/
	};
	//
	class KBEDATATYPE_INT16 : public KBEDATATYPE_BASE
	{
		public:

		//virtual int16 createFromStream(MemoryStream stream)
		//{
		//	return & ( stream.readInt16() );
		//}
		//
		//virtual void addToStream(KBundle stream, int16 v)
		//{
		//	stream.writeInt16( (int16) v  );
		//}
		//
		//virtual int16 parseDefaultValStr(std::string v)
		//{
		//	int16 ret = 0;
		//	//Int16.TryParse(v, out ret);
		//	ret = (int16) atoi(v.c_str());
		//	return  (object) ret;
		//}
	};
	class KBEDATATYPE_INT32 : public KBEDATATYPE_BASE
	{
		public:

		//virtual int32 createFromStream(MemoryStream stream)
		//{
		//	return  & (stream.readInt32());
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	stream.writeInt32((int32)v);//Convert.ToInt32(v));
		//}
		//
		//virtual int32 parseDefaultValStr(string v)
		//{
		//	int32 ret = 0;
		//	//Int32.TryParse(v, out ret);
		//	ret = (int32) atoi(v.c_str() );
		//	return  ret ;
		//}
	};
	class KBEDATATYPE_INT64 : public	 KBEDATATYPE_BASE
	{
		//public:

		//virtual int64 createFromStream(MemoryStream stream)
		//{
		//	return & (stream.readInt64());
		//}
		//
		//virtual void addToStream(KBundle stream, int64 v)
		//{
		//	stream.writeInt64( (int64) v );
		//}
		//
		//virtual int64 parseDefaultValStr(string v)
		//{
		//	int64 ret = 0;
		//	//Int64.TryParse(v, out ret);
		//	ret = (int64) atoi( v.c_str());
		//	return  ret;
		//}
	};

	class KBEDATATYPE_UINT8 : public KBEDATATYPE_BASE
	{
		//virtual uint8 createFromStream(MemoryStream stream)
		//{
		//	return  stream.readUint8();
		//}
		//
		//virtual void addToStream(KBundle stream, uint8 v)
		//{
		//	stream.writeUint8((uint8)v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	uint8 ret = 0;
		//	//Int64.TryParse(v, out ret);
		//	ret = (uint8) atoi( v.c_str());
		//	return & ret;
		//}
	};
	class KBEDATATYPE_UINT16 : public KBEDATATYPE_BASE
	{
		public:

		//virtual object createFromStream(MemoryStream stream)
		//{
		//	return  & stream.readUint16();
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	stream.writeUint16((uint16)v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	uint16 ret = 0;
		//	//Int64.TryParse(v, out ret);
		//	ret = (uint16) atoi( v.c_str());
		//	return  & ret;
		//}
	};
	class KBEDATATYPE_UINT32 : public KBEDATATYPE_BASE
	{
		public:

		//virtual object createFromStream(MemoryStream stream)
		//{
		//	return  & stream.readUint32();
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	stream.writeUint32((uint32)v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	uint32 ret = 0;
		//	//Int64.TryParse(v, out ret);
		//	ret = (uint32) atoi( v.c_str());
		//	return &ret;
		//}
	};
	class KBEDATATYPE_UINT64 : public KBEDATATYPE_BASE
	{
		public:

		//virtual object createFromStream(MemoryStream stream)
		//{
		//	return  & stream.readUint64();
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	stream.writeUint16((uint64)v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	uint16 ret = 0;
		//	//Int64.TryParse(v, out ret);
		//	ret = (uint64) atoi( v.c_str());
		//	return  & ret;
		//}
	};

	class KBEDATATYPE_FLOAT : public KBEDATATYPE_BASE
	{
		public:

		//virtual object createFromStream(MemoryStream& stream)
		//{
		//	return  & stream.readFloat();
		//}
		//
		//virtual void addToStream(KBundle& stream, object v)
		//{
		//	//stream.writeFloat((float)Convert.ToDouble(v));
		//	stream.writeFloat((float)v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	float ret = 0.0f;
		//	//float.TryParse(v, out ret);
		//	return & ret;
		//}
	};

	//class KBEDATATYPE_FLOAT : public KBEDATATYPE_BASE
	//{
	//	virtual object createFromStream(MemoryStream* stream)
	//	{
	//		return  (object) stream.readFloat();
	//	}
	//	
	//	virtual void addToStream(KBundle stream, object v)
	//	{
	//		stream.writeFloat((float)v);
	//	}
	//	
	//	virtual object parseDefaultValStr(string v)
	//	{
	//		float ret = 0.0f;
	//		//float.TryParse(v, out ret);
	//		ret = (float) v;
	//		return  (object) ret;
	//	}
	//};

	class KBEDATATYPE_DOUBLE : public KBEDATATYPE_BASE
	{
		public:

		//virtual object createFromStream(MemoryStream stream)
		//{
		//	return  & stream.read<double>();
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	stream.writeDouble((double)v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	float ret = 0.0f;
		//	//float.TryParse(v, out ret);
		//	ret = (double) v;
		//	return  & ret;
		//}
	};
	class KBEDATATYPE_STRING : public KBEDATATYPE_BASE
	{
	public:
		//virtual object createFromStream(MemoryStream stream)
		//{
		//	return & stream.readString();
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	stream.writeString( (string) v );//Convert.ToString(v));
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	return & v;
		//}
	};
	////
	class KBEDATATYPE_VECTOR2 : public KBEDATATYPE_BASE
	{
	public:

		//virtual object createFromStream(MemoryStream& stream)
		//{
		//	uint32 size = stream.readUint32();
		//	if(2 != size)
		//	{
		//		//Dbg.ERROR_MSG(string.Format("KBEDATATYPE_VECTOR2::createFromStream: size({0}) is error!", size));
		//	}
		//	
		//	Vector2 v;
		//	v.x = stream.readFloat();
		//	v.y = stream.readFloat();
		//	return &v;// new Vector2(stream.readFloat(), stream.readFloat());
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	Vector2 v_ = (Vector2*)v;
		//	stream.writeUint32(2);
 	//		stream.writeFloat( v_.x);
		//	stream.writeFloat( v_.y);
		//}
		
		//virtual object parseDefaultValStr(string v)
		//{
		//	Vector2 v1;
		//	return  & v1;//new Vector2(0.0f, 0.0f);
		//}
	};

	class KBEDATATYPE_VECTOR3 : public KBEDATATYPE_BASE
	{
	public:

		//virtual object createFromStream(MemoryStream stream)
		//{
		//	uint32 size = stream.readUint32();
		//	if(3 != size)
		//	{
		//		//Dbg.ERROR_MSG(string.Format("KBEDATATYPE_VECTOR3::createFromStream: size({0}) is error!", size));
		//	}
		//	
		//	Vector3 v;
		//	v.x = stream.readFloat();
		//	v.y = stream.readFloat();
		//	v.z = stream.readFloat();
		//	return  &v;
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	Vector3 v_ = (Vector3*)v;
		//	stream.writeUint32(3);
		//	stream.writeFloat( v_.x );
		//	stream.writeFloat( v_.y );
		//	stream.writeFloat( v_.z );
		////*	stream.writeFloat(((Vector3)v).x);
		//	stream.writeFloat(((Vector3)v).y);
		//	stream.writeFloat(((Vector3)v).z);*/
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	Vector3 v;
		//	return & v;//new Vector3(0.0f, 0.0f, 0.0f);
		//}
	};

	class KBEDATATYPE_VECTOR4 : public KBEDATATYPE_BASE
	{
	public:

		//virtual object createFromStream(MemoryStream stream)
		//{
		//	uint32 size = stream.readUint32();
		//	if(4 != size)
		//	{
		//		//Dbg.ERROR_MSG(string.Format("KBEDATATYPE_VECTOR4::createFromStream: size({0}) is error!", size));
		//	}
		//	
		//	return 0;//new Vector4(stream.readFloat(), stream.readFloat(), stream.readFloat(), stream.readFloat());
		//}
		//
		//virtual void addToStream(KBundle& stream, object v)
		//{
		//	stream.writeUint32(4);
		//	stream.writeFloat( 0.0f);
		//	stream.writeFloat( 0.0f);
		//	stream.writeFloat( 0.0f);
		//	stream.writeFloat( 0.0f);
		////*	stream.writeFloat(((Vector4)v).x);
		//	stream.writeFloat(((Vector4)v).y);
		//	stream.writeFloat(((Vector4)v).z);
		//	stream.writeFloat(((Vector4)v).w);*/
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	return 0;//new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		//}
	};

	////
	class KBEDATATYPE_PYTHON : public KBEDATATYPE_BASE
	{
	public:

		//virtual object createFromStream(MemoryStream& stream)
		//{
		//	std::string python;
		//	stream.readBlob(python);
		//	return &python;
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	//stream.writeBlob((byte[])v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	return 0;//new byte[0];
		//}
	};
	//
	class KBEDATATYPE_UNICODE : public KBEDATATYPE_BASE
	{
	public:
		//virtual object createFromStream(MemoryStream stream)
		//{
		//	//return System.Text.Encoding.UTF8.GetString(stream.readBlob());
		//}
		//
		//virtual void addToStream(KBundle& stream, object v)
		//{
		//	//stream.writeBlob(System.Text.Encoding.UTF8.GetBytes((string)v));
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	return 0;//v;
		//}
	};
	//
	class KBEDATATYPE_MAILBOX : public KBEDATATYPE_BASE
	{
	public:

		//virtual object createFromStream(MemoryStream stream)
		//{
		//	return (object) stream.readBlob();
		//}
		//
		//virtual void addToStream(Bundle stream, object v)
		//{
		//	//stream.writeBlob((byte[])v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	return 0;//new byte[0];
		//}
	};
	//
	class KBEDATATYPE_BLOB : public  KBEDATATYPE_BASE
	{
		//virtual object createFromStream(MemoryStream stream)
		//{
		//	return (object) stream.readBlob();
		//}
		//
		//virtual void addToStream(KBundle& stream, object v)
		//{
		//	//stream.writeBlob((byte[])v);
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	return 0;//new byte[0];
		//}
	};
	//
	class KBEDATATYPE_ARRAY : public KBEDATATYPE_BASE
	{
	public:
		//uint16 type;
		//object type;
		//
		virtual void bind();
		//{
		////*	if(type.GetType() == typeof(KBEDATATYPE_BASE).GetType())
		//		((KBEDATATYPE_BASE)type).bind();
		//	else
		//		if(EntityDef.iddatatypes.ContainsKey((UInt16)type))
		//			type = EntityDef.iddatatypes[(UInt16)type];*/
		//}
		//
		//virtual object createFromStream(MemoryStream stream)
		//{
		//	uint32 size = stream.readUint32();
		//	//List<object> datas = new List<object>();
		//	std::vector<object> datas;
		//	while(size > 0)
		//	{
		//		size--;
		//		//datas.Add(((KBEDATATYPE_BASE)type).createFromStream(stream));
		//		//datas.push_back( ( (KBEDATATYPE_BASE)type).createFromStream(stream) );
		//	};
		//	
		//	return (object) datas;
		//}
		//
		//virtual void addToStream(KBundle stream, object v)
		//{
		//	//stream.writeUint32((UInt32)((List<object>)v).Count);
		//	//for(int i=0; i<((List<object>)v).Count; i++)
		//	//{
		//	//	((KBEDATATYPE_BASE)type).addToStream(stream, ((List<object>)v)[i]);
		//	//}
		//}
		//
		//virtual object parseDefaultValStr(string v)
		//{
		//	return 0;//new byte[0];
		//}
	};
	//
	class KBEDATATYPE_FIXED_DICT : public KBEDATATYPE_BASE
	{
		public:
			//string implementedBy = "";
			////public Dictionary<string, object> dicttype = new Dictionary<string, object>();
			//std::map<std::string,object> dictype;
		/*	KBEDATATYPE_FIXED_DICT(DATATYPE_UID did = 0){	did_=did; };
			KBEDATATYPE_FIXED_DICT(KBEDATATYPE_FIXED_DICT& other)
			{ did_= other.did_; name = other.name; dicttype = other.dicttype; implementedBy = other.implementedBy;} ;
			KBEDATATYPE_FIXED_DICT(KBEDATATYPE_FIXED_DICT* other)
			{ did_= other->did_; name = other->name; dicttype = other->dicttype; implementedBy = other->implementedBy;} ;*/
			void bind();
		
			//virtual object createFromStream(MemoryStream& stream)
			//{
			////*	Dictionary<string, object> datas = new Dictionary<string, object>();
			//	foreach(string itemkey in dicttype.Keys)
			//	{
			//		datas[itemkey] = ((KBEDATATYPE_BASE)dicttype[itemkey]).createFromStream(stream);
			//	}
			//
			//	return datas;*/

			//	return 0;
			//}
		
			//virtual void addToStream(Bundle stream, object v)
			//{
			////*	foreach(string itemkey in dicttype.Keys)
			//	{
			//		((KBEDATATYPE_BASE)dicttype[itemkey]).addToStream(stream, ((Dictionary<string, object>)v)[itemkey]);
			//	}*/
			//}
		
			//virtual object parseDefaultValStr(string v)
			//{
			//	/*Dictionary<string, object> datas = new Dictionary<string, object>();
			//	foreach(string itemkey in dicttype.Keys)
			//	{
			//		datas[itemkey] = ((KBEDATATYPE_BASE)dicttype[itemkey]).parseDefaultValStr("");
			//	}
			//
			//	return datas;*/
			//	return 0;
			//}
			std::map<std::string,uint16> dicttype;
			std::string implementedBy;
			//std::string name;
	};
}
#endif /* defined(__libKBEClient__KDataTypes__) */
