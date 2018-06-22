// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_TELNET_SERVER_H
#define KBE_TELNET_SERVER_H
	
#include "common/common.h"
#include "helper/debug_helper.h"
#include "network/address.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"

namespace KBEngine{
namespace Network{
}

namespace script{
	class Script;
}

class TelnetHandler;

class TelnetServer : public Network::InputNotificationHandler
{
public:
    TelnetServer(Network::EventDispatcher* pDispatcher, Network::NetworkInterface* networkInterface);
	virtual ~TelnetServer(void);
	
	typedef std::map<int, KBEShared_ptr< TelnetHandler > >	TelnetHandlers;

	bool start(std::string passwd, std::string deflayer, u_int16_t port = 0, u_int32_t ip = INADDR_ANY);
	bool stop();

	void onTelnetHandlerClosed(int fd, TelnetHandler* pTelnetHandler);

	INLINE script::Script* pScript() const;
	INLINE void pScript(script::Script* p);

	INLINE std::string passwd();
	INLINE int deflayer();

	INLINE Network::NetworkInterface* pNetworkInterface() const;

	void closeHandler(int fd, TelnetHandler* pTelnetHandler);

	INLINE uint32 port();

private:
	int	handleInputNotification(int fd);

	TelnetHandlers handlers_;

	Network::EndPoint			listener_;
	Network::EventDispatcher*	pDispatcher_;

	script::Script* pScript_;

	std::string passwd_;
	int deflayer_;

	Network::NetworkInterface* pNetworkInterface_;

	uint32 port_;
};


}

#ifdef CODE_INLINE
#include "telnet_server.inl"
#endif
#endif // KBE_TELNET_SERVER_H
