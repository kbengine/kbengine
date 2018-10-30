// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SOCKETUDPPACKET_H
#define KBE_SOCKETUDPPACKET_H
	
#include "network/packet.h"
#include "common/objectpool.h"
	
namespace KBEngine{
namespace Network
{
class UDPPacket : public Packet
{
public:
	typedef KBEShared_ptr< SmartPoolObject< UDPPacket > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj(const std::string& logPoint);
	static ObjectPool<UDPPacket>& ObjPool();
	static UDPPacket* createPoolObject(const std::string& logPoint);
	static void reclaimPoolObject(UDPPacket* obj);
	static void destroyObjPool();
	static size_t maxBufferSize();

    UDPPacket(MessageID msgID = 0, size_t res = 0);
	virtual ~UDPPacket(void);
	
	int recvFromEndPoint(EndPoint & ep, Address* pAddr = NULL);

	virtual void onReclaimObject();
};

typedef SmartPointer<UDPPacket> UDPPacketPtr;
}
}

#endif // KBE_SOCKETUDPPACKET_H
