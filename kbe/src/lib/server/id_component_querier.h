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

#ifndef KBE_GET_COMPONENT_INFO_H
#define KBE_GET_COMPONENT_INFO_H

#include "common/common.h"
#include "network/common.h"
#include "network/bundle.h"
#include "network/endpoint.h"
#include "network/message_handler.h"

namespace KBEngine {

class IDComponentQuerier : public Network::Bundle
{
public:
	IDComponentQuerier();
	~IDComponentQuerier();

	void close();

	COMPONENT_ID query(COMPONENT_TYPE componentType, int32 uid);

	bool broadcast(uint16 port = 0);
	bool receive(Network::MessageArgs* recvArgs, sockaddr_in* psin = NULL, int32 timeout = 100000, bool showerr = true);

	bool good() const {
		return epListen_.good() && good_; 
	}

private:
	void send(COMPONENT_TYPE componentType, int32 uid);

private:
	Network::EndPoint epListen_, epBroadcast_;
	uint32 recvWindowSize_;
	int8 itry_;
	bool good_;
};

}

#endif
