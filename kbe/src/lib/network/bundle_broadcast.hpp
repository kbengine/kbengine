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

#ifndef KBE_BUNDLE_BROADCAST_HPP
#define KBE_BUNDLE_BROADCAST_HPP

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "message_handler.hpp"
#include "network/bundle.hpp"
#include "network/endpoint.hpp"

namespace KBEngine { 
namespace Mercury
{
class NetworkInterface;

/*
	���Է���Ĵ�����:��������ڹ㲥ĳЩ��Ϣ�� �������ռ������Ϣ��
*/
class BundleBroadcast : public Bundle
{
public:
	BundleBroadcast(NetworkInterface & networkInterface, uint16 bindPort = KBE_PORT_BROADCAST_DISCOVERY, 
		uint32 recvWindowSize = PACKET_MAX_SIZE_UDP);
	virtual ~BundleBroadcast();

	EventDispatcher& dispatcher();
	
	bool broadcast(uint16 port = 0);
	bool receive(MessageArgs* recvArgs, sockaddr_in* psin = NULL, int32 timeout = 100000, bool showerr = true);

	Mercury::EndPoint& epListen() { return epListen_; }

	void close();

	bool good()const { return epListen_.good() && good_; }

	void itry(int8 i){ itry_ = i; }
protected:
	Mercury::EndPoint epListen_, epBroadcast_;
	NetworkInterface & networkInterface_;
	uint32 recvWindowSize_;
	bool good_;
	int8 itry_;
};

}
}

#ifdef CODE_INLINE
#include "bundle_broadcast.ipp"
#endif
#endif // KBE_BUNDLE_BROADCAST_HPP
