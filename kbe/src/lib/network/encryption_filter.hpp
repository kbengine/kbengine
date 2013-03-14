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
#include "cstdkbe/blowfish.hpp"
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

class BlowfishFilter : public EncryptionFilter, public KBEBlowfish
{
public:

	virtual ~BlowfishFilter();
	BlowfishFilter(const Key & key);
	
	virtual Reason send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket);

	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);

	void encrypt(Packet * pInPacket, Packet * pOutPacket);
	void decrypt(Packet * pInPacket, Packet * pOutPacket);
private:
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
