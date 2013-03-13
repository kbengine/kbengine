/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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


#ifndef __ENCRYPTION_FILTER_HPP__
#define __ENCRYPTION_FILTER_HPP__

#include "network/packet_filter.hpp"

#ifdef USE_OPENSSL
#include "openssl/blowfish.h"
#endif

namespace KBEngine { 
namespace Mercury
{

class EncryptionFilter : public PacketFilter
{
public:
	virtual ~EncryptionFilter() {}

	virtual void encrypt(Packet * pInPacket, Packet * pOutPacket) = 0;
	virtual void decrypt(Packet * pInPacket, Packet * pOutPacket) = 0;
};

#ifdef USE_OPENSSL

class BlowfishFilter : public EncryptionFilter
{
public:
	static const int BLOCK_SIZE = 64 / 8;

	static const int MIN_KEY_SIZE = 32 / 8;
	static const int MAX_KEY_SIZE = 448 / 8;
	static const int DEFAULT_KEY_SIZE = 128 / 8;

	typedef std::string Key;

	virtual ~BlowfishFilter();
	BlowfishFilter(const Key & key);
	
	const Key & key() const { return key_; }
	const char * readableKey() const;
	bool isGood() const { return isGood_; }

	virtual Reason send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket);

	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);

	void encrypt(Packet * pInPacket, Packet * pOutPacket);
	void decrypt(Packet * pInPacket, Packet * pOutPacket);

	BF_KEY * pBFKey() { return (BF_KEY*)pBFKey_; }
private:
	bool initKey();

	int encrypt(const unsigned char * src, unsigned char * dest, int length);
	int decrypt(const unsigned char * src, unsigned char * dest, int length);

	Key key_;
	int keySize_;
	bool isGood_;

	void * pBFKey_;
};

#else

class BlowfishFilter : public EncryptionFilter
{
	virtual ~BlowfishFilter() {}
	void encrypt(Packet * pInPacket, Packet * pOutPacket){}
	void decrypt(Packet * pInPacket, Packet * pOutPacket){}
};

#endif

typedef SmartPointer<BlowfishFilter> BlowfishFilterPtr;

}
}

#endif // __ENCRYPTION_FILTER_HPP__
