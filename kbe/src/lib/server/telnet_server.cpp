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


#include "telnet_server.hpp"
#include "telnet_handler.hpp"
#include "network/bundle.hpp"
#include "network/endpoint.hpp"

#ifndef CODE_INLINE
#include "telnet_server.ipp"
#endif

namespace KBEngine { 

//-------------------------------------------------------------------------------------
TelnetServer::TelnetServer(Mercury::EventDispatcher* pDispatcher, Mercury::NetworkInterface* networkInterface):
handlers_(),
listener_(),
pDispatcher_(pDispatcher),
passwd_(),
deflayer_(TelnetHandler::TELNET_STATE_ROOT),
pNetworkInterface_(networkInterface)
{
}

//-------------------------------------------------------------------------------------
TelnetServer::~TelnetServer(void)
{
}

//-------------------------------------------------------------------------------------
bool TelnetServer::start(std::string passwd, std::string deflayer, u_int16_t port, u_int32_t ip)
{
	listener_.socket(SOCK_STREAM);
	listener_.setnonblocking(true);
	
	passwd_ = passwd;

	if(deflayer == "python")
		deflayer_ = TelnetHandler::TELNET_STATE_PYTHON;

	int tryn = 0;
	while(true)
	{
		if (listener_.bind(htons(port), htonl(ip)) == -1)
		{
			port++;
			tryn++;

			if(tryn > 65535)
			{
				tryn = 0;
				break;
			}

			continue;
		}
		else
		{
			tryn = 1;
			break;
		}
	};
	
	if(tryn == 0)
	{
		if (listener_.bind(htons(0), ip) == -1)
		{
			ERROR_MSG(boost::format("TelnetServer::start:: bind port(%1%) is failed! ip=%2%\n") % 
				0 % ip);

			return false;
		}
	}

	if(listener_.listen(1) == -1)
	{
		ERROR_MSG(boost::format("TelnetServer::start:: listen is failed! addr=%1%\n") % 
			listener_.c_str());

		return false;
	}

	if(!pDispatcher_->registerFileDescriptor(listener_, this))
	{
		ERROR_MSG(boost::format("TelnetServer::start:: registerFileDescriptor is failed! addr=%1%\n") % 
			listener_.c_str());

		return false;
	}

	INFO_MSG(boost::format("TelnetServer server is running on port %1%\n") % port);
	return true;
}

//-------------------------------------------------------------------------------------
void TelnetServer::onTelnetHandlerClosed(int fd, TelnetHandler* pTelnetHandler)
{
	INFO_MSG(boost::format("TelnetServer::onTelnetHandlerClosed: del handler(%1%)!\n") %
		pTelnetHandler->pEndPoint()->c_str());

	KBE_ASSERT(fd == (*pTelnetHandler->pEndPoint()));
	pDispatcher_->deregisterFileDescriptor(fd);
	handlers_.erase(fd);
}

//-------------------------------------------------------------------------------------
bool TelnetServer::stop()
{
	pDispatcher_->deregisterFileDescriptor(listener_);
	listener_.close();
	listener_.detach();
	return true;
}

//-------------------------------------------------------------------------------------
int	TelnetServer::handleInputNotification(int fd)
{
	KBE_ASSERT(listener_ == fd);

	int tickcount = 0;

	while(tickcount ++ < 1024)
	{
		Mercury::EndPoint* pNewEndPoint = listener_.accept();
		if(pNewEndPoint == NULL){

			if(tickcount == 1)
			{
				WARNING_MSG(boost::format("TelnetServer::handleInputNotification: accept endpoint(%1%) %2%!\n") %
					 fd % kbe_strerror());
			}

			break;
		}
		else
		{
			TelnetHandler* pTelnetHandler = new TelnetHandler(pNewEndPoint, this, passwd_.size() > 0 ? 
				TelnetHandler::TELNET_STATE_PASSWD : (TelnetHandler::TELNET_STATE)this->deflayer());

			if(!pDispatcher_->registerFileDescriptor((*pNewEndPoint), pTelnetHandler))
			{
				ERROR_MSG(boost::format("TelnetServer::start:: registerFileDescriptor(pTelnetHandler) is failed! addr=%1%\n") % 
					pNewEndPoint->c_str());
				
				delete pTelnetHandler;
				continue;
			}

			INFO_MSG(boost::format("TelnetServer::handleInputNotification: new handler(%1%)!\n") %
				pNewEndPoint->c_str());

			handlers_[fd].reset(pTelnetHandler);

			std::string s;

			if(passwd_.size() > 0)
			{
				s = "password:";
			}
			else
			{
				s = pTelnetHandler->getWelcome();
			}

			pNewEndPoint->send(s.c_str(), s.size());
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------
}

