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

#include "websocket_protocol.hpp"
#include "cstdkbe/memorystream.hpp"
#include "network/channel.hpp"

namespace KBEngine{
namespace html5{

//-------------------------------------------------------------------------------------
bool WebSocketProtocol::isWebSocketProtocol(MemoryStream* s)
{
	KBE_ASSERT(s != NULL);
	std::string data;
	(*s) >> data;

	std::vector<std::string> header_and_data;
//	KBEngine::kbe_split<char>(data.c_str, "\r\n\r\n", header_and_data);

	std::vector<std::string> headers;
	std::vector<std::string> values;
//	KBEngine::kbe_split<char>(header_and_data[0], "\r\n", values);
	return true;
}

//-------------------------------------------------------------------------------------
bool WebSocketProtocol::handshake(Mercury::Channel* pChannel, MemoryStream* s)
{
	KBE_ASSERT(s != NULL);
	return true;
}

//-------------------------------------------------------------------------------------
}
}

