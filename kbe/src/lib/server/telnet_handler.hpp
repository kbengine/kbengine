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

#ifndef __TELNET_HANDLER_H__
#define __TELNET_HANDLER_H__
	
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/interfaces.hpp"

namespace KBEngine{
namespace Mercury{
	class EndPoint;
}

class TelnetServer;
class TelnetHandler : public Mercury::InputNotificationHandler
{
public:
	enum TELNET_STATE
	{
		TELNET_STATE_PASSWD,
		TELNET_STATE_ROOT,
		TELNET_STATE_PYTHON
	};

    TelnetHandler(Mercury::EndPoint* pEndPoint, TelnetServer* pTelnetServer, 
		TELNET_STATE defstate = TELNET_STATE_ROOT);

	virtual ~TelnetHandler(void);
	
	INLINE Mercury::EndPoint* pEndPoint()const;

	std::string help();
	std::string getWelcome();
private:

	int	handleInputNotification(int fd);
	void onRecvInput();
	void processCommand();
	void processPythonCommand();

	bool checkUDLR();

	void sendEnter();
	void sendDelChar();
	void sendNewLine();
	void resetStartPosition();

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
};


}

#ifdef CODE_INLINE
#include "telnet_handler.ipp"
#endif
#endif
