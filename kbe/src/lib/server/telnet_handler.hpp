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

#ifndef KBE_TELNET_HANDLER_HPP
#define KBE_TELNET_HANDLER_HPP

#include "helper/profile_handler.hpp"
#include "pyscript/pyprofile_handler.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/interfaces.hpp"

namespace KBEngine{
namespace Mercury{
	class EndPoint;
	class NetworkInterface;
}

class TelnetHandler;
class TelnetProfileHandler
{
public:
	TelnetProfileHandler(TelnetHandler* pTelnetHandler):
	pTelnetHandler_(pTelnetHandler),
	isDestroyed_(false)
	{
	}

	virtual ~TelnetProfileHandler(){}

	void destroy(){ isDestroyed_ = true; }
protected:
	TelnetHandler* pTelnetHandler_;
	bool isDestroyed_;
};

class TelnetServer;
class TelnetHandler : public Mercury::InputNotificationHandler
{
public:
	enum TELNET_STATE
	{
		TELNET_STATE_PASSWD,
		TELNET_STATE_ROOT,
		TELNET_STATE_PYTHON,
		TELNET_STATE_READONLY,
		TELNET_STATE_QUIT
	};

    TelnetHandler(Mercury::EndPoint* pEndPoint, TelnetServer* pTelnetServer, Mercury::NetworkInterface* pNetworkInterface,
		TELNET_STATE defstate = TELNET_STATE_ROOT);

	virtual ~TelnetHandler(void);
	
	INLINE Mercury::EndPoint* pEndPoint()const;
	
	void setReadWrite();
	void readonly();

	std::string help();
	std::string getWelcome();

	void sendEnter();
	void sendDelChar();
	void sendNewLine();
	void resetStartPosition();

	void onProfileEnd(const std::string& datas);
private:
	void checkAfterStr();

	int	handleInputNotification(int fd);
	void onRecvInput();
	bool processCommand();
	void processPythonCommand(std::string command);

	bool checkUDLR();

	std::string getInputStartString();

	void historyCommandCheck();
	std::string getHistoryCommand(bool isNextCommand);

	std::deque<unsigned char> buffer_;
	std::deque<std::string> historyCommand_;
	int8 historyCommandIndex_;

	std::string command_;
	Mercury::EndPoint* pEndPoint_;
	TelnetServer* pTelnetServer_;

	uint8 state_;

	int32 currPos_;

	TelnetProfileHandler* pProfileHandler_;

	Mercury::NetworkInterface* pNetworkInterface_;

	bool getingHistroyCmd_;
};


class TelnetPyProfileHandler : public TelnetProfileHandler, public PyProfileHandler
{
public:
	TelnetPyProfileHandler(TelnetHandler* pTelnetHandler, Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr):
	TelnetProfileHandler(pTelnetHandler),
	PyProfileHandler(networkInterface, timinglen, name, addr)
	{
	}

	virtual ~TelnetPyProfileHandler(){}

	void sendStream(MemoryStream* s);
};

class TelnetCProfileHandler : public TelnetProfileHandler, public CProfileHandler
{
public:
	TelnetCProfileHandler(TelnetHandler* pTelnetHandler, Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr):
	TelnetProfileHandler(pTelnetHandler),
	CProfileHandler(networkInterface, timinglen, name, addr)
	{
	}

	virtual ~TelnetCProfileHandler(){}

	void sendStream(MemoryStream* s);
};

class TelnetEventProfileHandler : public TelnetProfileHandler, public EventProfileHandler
{
public:
	TelnetEventProfileHandler(TelnetHandler* pTelnetHandler, Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr):
	TelnetProfileHandler(pTelnetHandler),
	EventProfileHandler(networkInterface, timinglen, name, addr)
	{
	}

	virtual ~TelnetEventProfileHandler(){}

	void sendStream(MemoryStream* s);
};

class TelnetMercuryProfileHandler : public TelnetProfileHandler, public MercuryProfileHandler
{
public:
	TelnetMercuryProfileHandler(TelnetHandler* pTelnetHandler, Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr):
	TelnetProfileHandler(pTelnetHandler),
	MercuryProfileHandler(networkInterface, timinglen, name, addr)
	{
	}

	virtual ~TelnetMercuryProfileHandler(){}

	void sendStream(MemoryStream* s);
};

}

#ifdef CODE_INLINE
#include "telnet_handler.ipp"
#endif
#endif // KBE_TELNET_HANDLER_HPP
