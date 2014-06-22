//
//  KBundle.h
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#ifndef __libKBEClient__KBundle__
#define __libKBEClient__KBundle__

using namespace std;

#include <iostream>
#include <vector>
#include "KMemoryStream.h"
#include "KMessage.h"
#include "KNetworkInterface.h"


namespace KBEngineClient{

	class KMessage;
	class MemoryStream;

	class KBundle{
	
	public:

		MemoryStream* streamPtr;
		//MemoryStream& stream;
		std::vector<MemoryStream*> streamList;
		int numMessage;
		int messageLength;
		//KMessage& msgtype;
		KMessage* msgtypePtr;

		KBundle();
		~KBundle();
		void newmessage(KMessage& msg);
		void fini( bool param1 );
		void writeMsgLength();
		void writeUint16(uint16 v);
		void send(KNetworkInterface networkInterface);
		void checkStream(int v);
		void writeInt8(int8 v);
		void writeInt16(int16 v);
		void writeInt32(int32 v);
		void writeInt64(int64 v);
		void writeUint8(uint8 v);
		void writeUint32(uint32 v);
		void writeUint64(uint64 v);
		void writeFloat(float v);
		void writeDouble(double v);
		void writeString(std::string v);
		void writeBlob(char* v);


	};

}
#endif /* defined(__libKBEClient__KBundle__) */
