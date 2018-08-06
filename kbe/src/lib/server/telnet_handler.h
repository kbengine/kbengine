// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_TELNET_HANDLER_H
#define KBE_TELNET_HANDLER_H

#include "helper/profile_handler.h"
#include "pyscript/pyprofile_handler.h"
#include "common/common.h"
#include "network/common.h"
#include "helper/debug_helper.h"
#include "network/interfaces.h"

namespace KBEngine{
namespace Network{
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

	virtual void destroy(){ isDestroyed_ = true; }

protected:
	TelnetHandler* pTelnetHandler_;
	bool isDestroyed_;
};

class TelnetServer;
class TelnetHandler : public Network::InputNotificationHandler
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

    TelnetHandler(Network::EndPoint* pEndPoint, TelnetServer* pTelnetServer, Network::NetworkInterface* pNetworkInterface,
		TELNET_STATE defstate = TELNET_STATE_ROOT);

	virtual ~TelnetHandler(void);
	
	INLINE Network::EndPoint* pEndPoint() const;
	
	void setReadWrite();
	void readonly();

	std::string help();
	std::string getWelcome();

	void sendEnter();
	void sendDelChar();
	void sendNewLine();
	void sendBackSpace();

	/** telnet协议
		IAC WILL SUPPRESS GO AHEAD, 抑制继续进行
	*/
	void sendWillSuppressGoAhead();

	void sendDOTT();

	/** telnet协议
		IAC DO 24, 同意询问终端类型
	*/
	void sendQueryClientTerminalType();

	/** telnet协议
		IAC SB 24 0'v' 't' '1' '0' '0' IAC SE, 发送服务端的终端类型
	*/
	void sendServerTerminalType();

	/** telnet协议
		IAC WILL ECHO, 回显
	*/
	void sendWillEcho();
	
	void resetStartPosition();

	void onProfileEnd(const std::string& datas);

private:
	Network::Reason checkLastErrors();

	void checkAfterStr();

	int	handleInputNotification(int fd);
	void onRecvInput(const char *buffer, int size);
	bool processCommand();
	void processPythonCommand(std::string command);
	void processBackSpace();

	bool checkUDLR(const std::string &cmd);
	void checkTerminalType(std::string &iac);

	std::string getInputStartString();

	void historyCommandCheck();
	std::string getHistoryCommand(bool isNextCommand);

	std::deque<std::string> historyCommand_;
	int8 historyCommandIndex_;

	std::string command_;
	Network::EndPoint* pEndPoint_;
	TelnetServer* pTelnetServer_;

	uint8 state_;

	int32 currPos_;

	TelnetProfileHandler* pProfileHandler_;

	Network::NetworkInterface* pNetworkInterface_;

	bool getingHistroyCmd_;

	int clientTermialType_;
};


class TelnetPyProfileHandler : public TelnetProfileHandler, public PyProfileHandler
{
public:
	TelnetPyProfileHandler(TelnetHandler* pTelnetHandler, Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr):
	TelnetProfileHandler(pTelnetHandler),
	PyProfileHandler(networkInterface, timinglen, name, addr)
	{
	}

	virtual ~TelnetPyProfileHandler(){}

	void sendStream(MemoryStream* s);
};

class TelnetPyTickProfileHandler : public TelnetProfileHandler, public PyTickProfileHandler
{
public:
	TelnetPyTickProfileHandler(TelnetHandler* pTelnetHandler, Network::NetworkInterface & networkInterface, uint32 timinglen,
		std::string name, const Network::Address& addr) :
		TelnetProfileHandler(pTelnetHandler),
		PyTickProfileHandler(networkInterface, timinglen, name, addr)
	{
	}

	virtual ~TelnetPyTickProfileHandler(){}

	void sendStream(MemoryStream* s);
	virtual void timeout();
	virtual void destroy();
};

class TelnetCProfileHandler : public TelnetProfileHandler, public CProfileHandler
{
public:
	TelnetCProfileHandler(TelnetHandler* pTelnetHandler, Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr):
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
	TelnetEventProfileHandler(TelnetHandler* pTelnetHandler, Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr):
	TelnetProfileHandler(pTelnetHandler),
	EventProfileHandler(networkInterface, timinglen, name, addr)
	{
	}

	virtual ~TelnetEventProfileHandler(){}

	void sendStream(MemoryStream* s);
};

class TelnetNetworkProfileHandler : public TelnetProfileHandler, public NetworkProfileHandler
{
public:
	TelnetNetworkProfileHandler(TelnetHandler* pTelnetHandler, Network::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Network::Address& addr):
	TelnetProfileHandler(pTelnetHandler),
	NetworkProfileHandler(networkInterface, timinglen, name, addr)
	{
	}

	virtual ~TelnetNetworkProfileHandler(){}

	void sendStream(MemoryStream* s);
};

}

#ifdef CODE_INLINE
#include "telnet_handler.inl"
#endif
#endif // KBE_TELNET_HANDLER_H
