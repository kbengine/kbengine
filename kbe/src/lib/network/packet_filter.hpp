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


#ifndef KBE_PACKET_FILTER_HPP
#define KBE_PACKET_FILTER_HPP

#include "network/common.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/refcountable.hpp"

namespace KBEngine { 
namespace Mercury
{

class Channel;
class NetworkInterface;
class Packet;
class Address;
class PacketFilter;
class PacketReceiver;


class PacketFilter : public RefCountable
{
public:
	virtual ~PacketFilter() {}

	virtual Reason send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket);

	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);
};

typedef SmartPointer<PacketFilter> PacketFilterPtr;

}
}

#ifdef CODE_INLINE
#include "packet_filter.ipp"
#endif

#endif // KBE_PACKET_FILTER_HPP
