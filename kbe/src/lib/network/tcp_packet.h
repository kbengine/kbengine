// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
	static SmartPoolObjectPtr createSmartPoolObj(const std::string& logPoint);
	static ObjectPool<TCPPacket>& ObjPool();
	static TCPPacket* createPoolObject(const std::string& logPoint);
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
