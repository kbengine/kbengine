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

#ifndef KBE_TELNET_SERVER_HPP
#define KBE_TELNET_SERVER_HPP
	
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "network/endpoint.hpp"
#include "network/event_dispatcher.hpp"

namespace KBEngine{
namespace Mercury{
}

namespace script{
	class Script;
}

class TelnetHandler;

class TelnetServer : public Mercury::InputNotificationHandler
{
public:
    TelnetServer(Mercury::EventDispatcher* pDispatcher, Mercury::NetworkInterface* networkInterface);
	virtual ~TelnetServer(void);
	
	typedef std::map<int, KBEShared_ptr< TelnetHandler > >	TelnetHandlers;

	bool start(std::string passwd, std::string deflayer, u_int16_t port = 0, u_int32_t ip = INADDR_ANY);
	bool stop();

	void onTelnetHandlerClosed(int fd, TelnetHandler* pTelnetHandler);

	INLINE script::Script* pScript()const;
	INLINE void pScript(script::Script* p);

	INLINE std::string passwd();
	INLINE int deflayer();

	INLINE Mercury::NetworkInterface* pNetworkInterface()const;

	void closeHandler(int fd, TelnetHandler* pTelnetHandler);

	INLINE uint32 port();
private:
	int	handleInputNotification(int fd);

	TelnetHandlers handlers_;

	Mercury::EndPoint			listener_;
	Mercury::EventDispatcher*	pDispatcher_;

	script::Script* pScript_;

	std::string passwd_;
	int deflayer_;

	Mercury::NetworkInterface* pNetworkInterface_;

	uint32 port_;
};


}

#ifdef CODE_INLINE
#include "telnet_server.ipp"
#endif
#endif // KBE_TELNET_SERVER_HPP
