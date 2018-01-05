/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KBE_SOCKETTCPPACKET_H
#define KBE_SOCKETTCPPACKET_H
	
// common include
#include "network/packet.h"
#include "common/objectpool.h"
	
namespace KBEngine{
namespace Network
{
class TCPPacket : public Packet
{
public:
	typedef KBEShared_ptr< SmartPoolObject< TCPPacket > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();
	static ObjectPool<TCPPacket>& ObjPool();
	static TCPPacket* createPoolObject();
	static void reclaimPoolObject(TCPPacket* obj);
	static void destroyObjPool();

	static size_t maxBufferSize();

    TCPPacket(MessageID msgID = 0, size_t res = 0);
	virtual ~TCPPacket(void);
	
	int recvFromEndPoint(EndPoint & ep, Address* pAddr = NULL);

	virtual void onReclaimObject();
};

typedef SmartPointer<TCPPacket> TCPPacketPtr;
}
}

#endif // KBE_SOCKETTCPPACKET_H
