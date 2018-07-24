// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_ENCRYPTION_FILTER_H
#define KBE_ENCRYPTION_FILTER_H

#include "network/packet_filter.h"

#ifdef USE_OPENSSL
#include "common/blowfish.h"
#endif

namespace KBEngine { 
namespace Network
{

class EncryptionFilter : public PacketFilter
{
public:
	virtual ~EncryptionFilter() {}

	virtual void encrypt(Packet * pInPacket, Packet * pOutPacket) = 0;
	virtual void decrypt(Packet * pInPacket, Packet * pOutPacket) = 0;
};



class BlowfishFilter : public EncryptionFilter, public KBEBlowfish
{
public:

	virtual ~BlowfishFilter();
	BlowfishFilter(const Key & key);
	BlowfishFilter();

	virtual Reason send(Channel * pChannel, PacketSender& sender, Packet * pPacket, int userarg);

	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);

	void encrypt(Packet * pInPacket, Packet * pOutPacket);
	void decrypt(Packet * pInPacket, Packet * pOutPacket);
private:
	Packet * pPacket_;
	Network::PacketLength packetLen_;
	uint8 padSize_;
};

typedef SmartPointer<BlowfishFilter> BlowfishFilterPtr;

inline EncryptionFilter* createEncryptionFilter(int8 type, const std::string& datas)
{
	EncryptionFilter* pEncryptionFilter = NULL;
	switch(type)
	{
	case 1:
		pEncryptionFilter = new BlowfishFilter(datas);
		break;
	default:
		break;
	}

	return pEncryptionFilter;
}

}
}

#endif // KBE_ENCRYPTION_FILTER_H
