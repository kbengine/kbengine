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

#include "loginapp.hpp"
#include "account_activate_handler.hpp"
#include "network/event_dispatcher.hpp"
#include "network/event_poller.hpp"
#include "network/endpoint.hpp"
#include "network/bundle.hpp"
#include "helper/debug_helper.hpp"

#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
AccountActivateHandler::AccountActivateHandler():
pEndPoint_(NULL),
clients_()
{
	pEndPoint_ = new Mercury::EndPoint();

	pEndPoint_->socket(SOCK_STREAM);

	if (!pEndPoint_->good())
	{
		ERROR_MSG("AccountActivateHandler::process: couldn't create a socket\n");
		return;
	}

	if (pEndPoint_->bind(htons(g_kbeSrvConfig.emailAtivationInfo_.cb_port), 
		Loginapp::getSingleton().getNetworkInterface().extaddr().ip) == -1)
	{
		ERROR_MSG(boost::format("AccountActivateHandler::bind(%1%): \n") %
			 kbe_strerror());

		pEndPoint_->close();
		return;
	}

	if(pEndPoint_->listen() == -1)
	{
		ERROR_MSG(boost::format("AccountActivateHandler::listeningSocket(%1%): \n") %
			 kbe_strerror());

		pEndPoint_->close();
		return;
	}

	pEndPoint_->setnonblocking(true);

	Loginapp::getSingleton().getNetworkInterface().dispatcher().registerFileDescriptor(*pEndPoint_, this);

	INFO_MSG(boost::format("AccountActivateHandler::bind: %1%:%2%\n") %
		inet_ntoa((struct in_addr&)Loginapp::getSingleton().getNetworkInterface().extaddr().ip) % 
		g_kbeSrvConfig.emailAtivationInfo_.cb_port);
}

//-------------------------------------------------------------------------------------
AccountActivateHandler::~AccountActivateHandler()
{
	clients_.clear();
	Loginapp::getSingleton().getNetworkInterface().dispatcher().deregisterFileDescriptor(*pEndPoint_);
	SAFE_RELEASE(pEndPoint_);
}

//-------------------------------------------------------------------------------------
int AccountActivateHandler::handleInputNotification(int fd)
{
	if(fd == *pEndPoint_)
	{
		u_int16_t port;
		u_int32_t addr;

		Mercury::EndPoint* newclient = pEndPoint_->accept(&port, &addr);

		if(newclient == NULL)
		{
			ERROR_MSG(boost::format("AccountActivateHandler::handleInputNotification: accept is error:%1%.\n") % kbe_strerror());
			return 0;
		}

		INFO_MSG(boost::format("AccountActivateHandler:handleInputNotification: newclient = %1%\n") %
			newclient->c_str());

		CLIENT& client = clients_[*newclient];
		client.endpoint = KBEShared_ptr< Mercury::EndPoint >(newclient);
		client.state = 0;
		Loginapp::getSingleton().getNetworkInterface().dispatcher().registerFileDescriptor(*newclient, this);
	}
	else
	{
		std::map< int, CLIENT >::iterator iter = clients_.find(fd);
		if(iter == clients_.end())
		{
			ERROR_MSG(boost::format("AccountActivateHandler:handleInputNotification: fd(%1%) not found!\n") %
				fd);
			return 0;
		}

		CLIENT& client = iter->second;
		Mercury::EndPoint* newclient = iter->second.endpoint.get();
		Loginapp::getSingleton().getNetworkInterface().dispatcher().deregisterFileDescriptor(*newclient);

		char buffer[1024];
		int len = newclient->recv(&buffer, 1024);

		if(len <= 0)
		{
			ERROR_MSG(boost::format("AccountActivateHandler:handleInputNotification: recv error, newclient = %1%\n") %
				newclient->c_str());

			if(client.state == 1)
				clients_.erase(iter);
			return 0;
		}

		if(client.state == 1)
			clients_.erase(iter);

		std::string s = buffer;
		std::string::size_type fi1 = s.find("accountactivate?");
		std::string::size_type fi2 = s.find(" HTTP/");

		std::string code;
		if(fi1 != std::string::npos && fi2 != std::string::npos)
		{
			int ilen = strlen("accountactivate?");
			code.assign(s.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
		}

		client.state = 1;

		if(code.size() > 0)
		{
			INFO_MSG(boost::format("AccountActivateHandler:handleInputNotification: code = %1%\n") %
				code.c_str());

			client.state = 2;
			client.code = code;

			Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
			Components::ComponentInfos* dbmgrinfos = NULL;

			if(cts.size() > 0)
				dbmgrinfos = &(*cts.begin());

			if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
			{
				return 0;
			}

			// œÚdbmgrº§ªÓ’À∫≈
			Mercury::Bundle bundle;
			bundle.newMessage(DbmgrInterface::accountActivate);
			bundle << code;
			bundle.send(Loginapp::getSingleton().getNetworkInterface(), dbmgrinfos->pChannel);
		}
		else
		{
			if(client.state != 2)
				clients_.erase(iter);
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------
}
