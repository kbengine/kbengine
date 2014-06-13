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
#include "cstdkbe/base64.hpp"
#include "cstdkbe/sha1.hpp"

#if KBE_PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
#pragma comment(lib, "libeay32_d.lib")
#pragma comment(lib, "ssleay32_d.lib")
#else
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif
#endif

namespace KBEngine{
namespace html5{

//-------------------------------------------------------------------------------------
bool WebSocketProtocol::isWebSocketProtocol(MemoryStream* s)
{
	KBE_ASSERT(s != NULL);

	std::string data;
	size_t rpos = s->rpos();
	size_t wpos = s->wpos();

	(*s) >> data;

	size_t fi = data.find_first_of("Sec-WebSocket-Key");
	if(fi == std::string::npos)
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	fi = data.find_first_of("GET");
	if(fi == std::string::npos)
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	std::vector<std::string> header_and_data;
	header_and_data = KBEngine::strutil::kbe_splits(data, "\r\n\r\n");
	
	if(header_and_data.size() != 2)
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	s->rpos(rpos);
	s->wpos(wpos);
	return true;
}

//-------------------------------------------------------------------------------------
bool WebSocketProtocol::handshake(Mercury::Channel* pChannel, MemoryStream* s)
{
	KBE_ASSERT(s != NULL);

	std::string data;
	size_t rpos = s->rpos();
	size_t wpos = s->wpos();

	(*s) >> data;

	std::vector<std::string> header_and_data;
	header_and_data = KBEngine::strutil::kbe_splits(data, "\r\n\r\n");
	
	if(header_and_data.size() != 2)
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	KBEUnordered_map<std::string, std::string> headers;
	std::vector<std::string> values;
	
	values = KBEngine::strutil::kbe_splits(header_and_data[0], "\r\n");
	std::vector<std::string>::iterator iter = values.begin();

	for(; iter != values.end(); iter++)
	{
		header_and_data = KBEngine::strutil::kbe_splits((*iter), ": ");

		if(header_and_data.size() == 2)
			headers[header_and_data[0]] = header_and_data[1];
	}

	std::string szKey, szOrigin, szHost;

	KBEUnordered_map<std::string, std::string>::iterator findIter = headers.find("Sec-WebSocket-Origin");
	if(findIter == headers.end())
	{
		findIter = headers.find("Origin");
		if(findIter == headers.end())
		{
			s->rpos(rpos);
			s->wpos(wpos);
			return false;
		}
	}

	szOrigin = findIter->second;

	findIter = headers.find("Sec-WebSocket-Key");
	if(findIter == headers.end())
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	szKey = findIter->second;

	findIter = headers.find("Host");
	if(findIter == headers.end())
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	szHost = findIter->second;


    std::string server_key = szKey;
    server_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	SHA1        sha;
	unsigned int    message_digest[5];

	sha.Reset();
	sha << server_key.c_str();

	sha.Result(message_digest);

	for (int i = 0; i < 5; i++) {
		message_digest[i] = htonl(message_digest[i]);
	}

    server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest), 20);

	std::string ackHandshake = "HTTP/1.1 101 Switching Protocols\r\n"
								"Upgrade:websocket\r\n"
								"Connection: Upgrade\r\n"
								"Sec-WebSocket-Accept:";

	ackHandshake += server_key; 
	ackHandshake += "\r\n";
	ackHandshake += "WebSocket-Origin:"; 
	ackHandshake += szOrigin; ackHandshake += "\r\n";
	ackHandshake += "WebSocket-Location: ws://";
	ackHandshake += szHost;
	ackHandshake += "/WebManagerSocket\r\n";
	ackHandshake += "WebSocket-Protocol:WebManagerSocket\r\n\r\n";

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle) << ackHandshake;
	(*pBundle).pCurrPacket()->wpos((*pBundle).pCurrPacket()->wpos() - 1);
	pChannel->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
}
}

