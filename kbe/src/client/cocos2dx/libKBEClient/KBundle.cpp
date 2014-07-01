//
//  KBundle.cpp
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#include "KBundle.h"
#include "KMessage.h"
#include "KNetworkInterface.h"
#include "cocos2d.h"


namespace KBEngineClient{

void KBundle::newmessage( KMessage& msg )
{
		
	fini(false);

	this->msgtypePtr = &msg;
	//this->msgtype =  msg;
	numMessage += 1;

	KMessage& msgtype = *( msgtypePtr);

	writeUint16(msgtype.id);

	if(msgtype.msglen == -1)
	{
		writeUint16(0);
		messageLength = 0;
	}
}

KBundle::KBundle():streamPtr(0),msgtypePtr(0),numMessage(0),messageLength(0)
{
	streamPtr = new MemoryStream();
	//cocos2d::CCLog(" default size= %d", streamPtr->size() );
}

KBundle::~KBundle()
{

}


void KBundle::send(KNetworkInterface networkInterface)
{
	fini(true);
			
	for(int i=0; i<streamList.size(); i++)
	{
		streamPtr = streamList[i];
		MemoryStream& stream = *streamPtr;
		networkInterface.send( stream.data() ,stream.size() );
	}
	//first to clear its content.
	streamList.clear();
	streamPtr = new MemoryStream();
}
		
void KBundle::checkStream(int v)
{
	if(!streamPtr || v > streamPtr->fillfree2() )
	{
		//return (UInt32)(BUFFER_MAX - wpos);
		if(streamPtr && v > streamPtr->fillfree2() )
			streamList.push_back(streamPtr);
		streamPtr = new MemoryStream();
	}
	
	messageLength += v;
}
		
//---------------------------------------------------------------------------------
void KBundle::writeInt8(int8 v)
{
	checkStream(1);
	//stream.writeInt8(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
	
void KBundle::writeInt16(int16 v)
{
	checkStream(2);
	//stream.writeInt16(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
			
void KBundle::writeInt32(int32 v)
{
	checkStream(4);
	//stream.writeInt32(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
	
void KBundle::writeInt64(int64 v)
{
	checkStream(8);
	//stream.writeInt64(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
		
void KBundle::writeUint8(uint8 v)
{
	checkStream(1);
	//stream.writeUint8(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
	
void KBundle::writeUint16(uint16 v)
{
	checkStream(2);
	//stream.writeUint16(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;

	//5,0 or 0,5?
}
			
void KBundle::writeUint32(uint32 v)
{
	checkStream(4);
	//stream.writeUint32(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
	
void KBundle::writeUint64(uint64 v)
{
	checkStream(8);
//	stream.writeUint64(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
		
void KBundle::writeFloat(float v)
{
	checkStream(4);
	//stream.writeFloat(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
	
void KBundle::writeDouble(double v)
{
	checkStream(8);
	//stream.writeDouble(v);
	MemoryStream& stream = *streamPtr;
	stream<<v;
}
		
void KBundle::writeString(const std::string v)
{
	//checkStream(v.Length + 1);
	//stream.writeString(v);/
	checkStream( v.length() + 1);
	MemoryStream& stream = *streamPtr;
	stream << v;
}
		
void KBundle::writeBlob(char* v,int size)
{
	checkStream( size + 4 );
	//stream.appendBlob(v);
	MemoryStream& stream = *streamPtr;
	stream<<(uint32)size;
	if(size > 0)
		stream<<v;
}//

void KBundle::fini(bool issend)
{
	if(numMessage > 0)
	{
		writeMsgLength();
		if(streamPtr != 0)
			streamList.push_back(streamPtr);
	}
			
	if(issend)
	{
		numMessage = 0;
		//delete msgtype;
		//msgtype = 0;
		msgtypePtr =0;
	}
}

void KBundle::writeMsgLength()
{

		KMessage& msgtype = *( msgtypePtr);

		if(msgtype.msglen != -1)
			return;

		//if not use & not correct set length. bug issue
		MemoryStream& stream = *(streamPtr);

		if(stream.opsize() >= messageLength)
		{
			int idx = (int)stream.opsize() - messageLength - 2;
			stream.data()[idx] = (uint8)(messageLength & 0xff);
			stream.data()[idx + 1] = (uint8)(messageLength >> 8 & 0xff);
		}
		else
		{
			int size = messageLength - (int)stream.opsize();
			CCAssert( streamList.size() > 0 ,"");
			uint8* data = streamList[numMessage - 1]->data();
				
			int idx = stream.size() - size - 2;
				
			data[idx] = (uint8)(messageLength & 0xff);
			data[idx + 1] = (uint8)(messageLength >> 8 & 0xff);
		}
}

void KBundle::writeStream( MemoryStream* stream )
{
	//push a stream. extend stream list,fix this issue when method has arguments. 
	checkStream( stream->opsize());
	this->streamPtr->append( stream->data() , stream->opsize() );
	//no need length for stream. 
}


} //end namespace.
