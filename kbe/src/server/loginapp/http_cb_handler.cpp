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
#include "http_cb_handler.hpp"
#include "network/event_dispatcher.hpp"
#include "network/event_poller.hpp"
#include "network/endpoint.hpp"
#include "network/bundle.hpp"
#include "network/http_utility.hpp"
#include "helper/debug_helper.hpp"
#include "server/serverconfig.hpp"

#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
HTTPCBHandler::HTTPCBHandler():
pEndPoint_(NULL),
clients_()
{
	pEndPoint_ = new Mercury::EndPoint();

	pEndPoint_->socket(SOCK_STREAM);

	if (!pEndPoint_->good())
	{
		ERROR_MSG("HTTPCBHandler::process: couldn't create a socket\n");
		return;
	}

	if (pEndPoint_->bind(htons(g_kbeSrvConfig.getLoginApp().http_cbport), 
		Loginapp::getSingleton().networkInterface().extaddr().ip) == -1)
	{
		ERROR_MSG(boost::format("HTTPCBHandler::bind(%1%): \n") %
			 kbe_strerror());

		pEndPoint_->close();
		return;
	}

	if(pEndPoint_->listen() == -1)
	{
		ERROR_MSG(boost::format("HTTPCBHandler::listeningSocket(%1%): \n") %
			 kbe_strerror());

		pEndPoint_->close();
		return;
	}

	pEndPoint_->setnonblocking(true);

	Loginapp::getSingleton().networkInterface().dispatcher().registerFileDescriptor(*pEndPoint_, this);

	INFO_MSG(boost::format("HTTPCBHandler::bind: %1%:%2%\n") %
		inet_ntoa((struct in_addr&)Loginapp::getSingleton().networkInterface().extaddr().ip) % 
		g_kbeSrvConfig.getLoginApp().http_cbport);
}

//-------------------------------------------------------------------------------------
HTTPCBHandler::~HTTPCBHandler()
{
	clients_.clear();
	Loginapp::getSingleton().networkInterface().dispatcher().deregisterFileDescriptor(*pEndPoint_);
	SAFE_RELEASE(pEndPoint_);
}

//-------------------------------------------------------------------------------------
int HTTPCBHandler::handleInputNotification(int fd)
{
	if(fd == *pEndPoint_)
	{
		u_int16_t port;
		u_int32_t addr;

		Mercury::EndPoint* newclient = pEndPoint_->accept(&port, &addr);

		if(newclient == NULL)
		{
			ERROR_MSG(boost::format("HTTPCBHandler::handleInputNotification: accept is error:%1%.\n") % kbe_strerror());
			return 0;
		}

		INFO_MSG(boost::format("HTTPCBHandler:handleInputNotification: newclient = %1%\n") %
			newclient->c_str());
		
		newclient->setnonblocking(true);
		CLIENT& client = clients_[*newclient];
		client.endpoint = KBEShared_ptr< Mercury::EndPoint >(newclient);
		client.state = 0;
		Loginapp::getSingleton().networkInterface().dispatcher().registerFileDescriptor(*newclient, this);
	}
	else
	{
		std::map< int, CLIENT >::iterator iter = clients_.find(fd);
		if(iter == clients_.end())
		{
			ERROR_MSG(boost::format("HTTPCBHandler:handleInputNotification: fd(%1%) not found!\n") %
				fd);
			return 0;
		}

		CLIENT& client = iter->second;
		Mercury::EndPoint* newclient = iter->second.endpoint.get();

		char buffer[1024];
		int len = newclient->recv(&buffer, 1024);

		if(len <= 0)
		{
			ERROR_MSG(boost::format("HTTPCBHandler:handleInputNotification: recv error, newclient = %1%, recv=%2%.\n") %
				newclient->c_str() % len);
		
			if(len == 0)
			{
				Loginapp::getSingleton().networkInterface().dispatcher().deregisterFileDescriptor(*newclient);
				clients_.erase(iter);
			}
			return 0;
		}

		if(client.state == 1)
		{
			Loginapp::getSingleton().networkInterface().dispatcher().deregisterFileDescriptor(*newclient);
			clients_.erase(iter);
		}

		int type = 0;
		std::string s = buffer;
		
		std::string keys = "<policy-file-request/>";
		std::string::size_type fi0 = s.find(keys);
		if(fi0 != std::string::npos)
		{
			if(client.state != 1)
			{
				std::string response = "<?xml version='1.0'?><cross-domain-policy><allow-access-from domain=""*"" to-ports=""*"" /></cross-domain-policy>";
				iter->second.endpoint->send(response.c_str(), response.size());
				Loginapp::getSingleton().networkInterface().dispatcher().deregisterFileDescriptor(*newclient);
				clients_.erase(iter);
			}

			return 0;
		}

		keys = "accountactivate_";
		std::string::size_type fi1 = s.find(keys);
		if(fi1 == std::string::npos)
		{
			keys = "resetpassword_";
			fi1 = s.find(keys);
			if(fi1 == std::string::npos)
			{
				keys = "bindmail_";
				fi1 = s.find(keys);
				if(fi1 != std::string::npos)
				{
					type = 3;
				}
			}
			else
			{
				type = 2;
			}
		}
		else
		{
			type = 1;
		}

		std::string::size_type fi2 = s.find("?");
		std::string::size_type fi3 = s.find(" HTTP/");

		if(fi2 == std::string::npos || fi2 > fi3)
			fi2 = fi3;

		if(fi2 <= fi1)
		{
			return 0;
		}

		std::string code;
		if(fi1 != std::string::npos && fi2 != std::string::npos)
		{
			int ilen = keys.size();
			code.assign(s.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
		}

		client.state = 1;
		
		code = KBEngine::strutil::kbe_trim(code);

		if(code.size() > 0)
		{
			INFO_MSG(boost::format("HTTPCBHandler:handleInputNotification: code = %1%\n") %
				code.c_str());

			client.code = code;

			Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
			Components::ComponentInfos* dbmgrinfos = NULL;

			if(cts.size() > 0)
				dbmgrinfos = &(*cts.begin());

			if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
			{
				return 0;
			}

			std::string hellomessage;

			if(type == 1)
			{
				// 向dbmgr激活账号
				Mercury::Bundle bundle;
				bundle.newMessage(DbmgrInterface::accountActivate);
				bundle << code;
				bundle.send(Loginapp::getSingleton().networkInterface(), dbmgrinfos->pChannel);

				hellomessage = g_kbeSrvConfig.emailAtivationInfo_.backlink_hello_message;
			}
			else if(type == 2)
			{
				std::string::size_type fi1 = s.find("password=");
				std::string::size_type fi2 = std::string::npos;
				
				if(fi1 != std::string::npos)
					fi2 = s.find("&", fi1);

				std::string password;
				
				if(fi1 != std::string::npos && fi2 != std::string::npos)
				{
					client.state = 2;
					if(fi1 < fi2)
					{
						int ilen = strlen("password=");
						password.assign(s.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
					}

					fi1 = s.find("username=");
					fi2 = std::string::npos;
				
					if(fi1 != std::string::npos)
						fi2 = s.find("&", fi1);

					std::string username;
					
					if(fi1 != std::string::npos && fi2 != std::string::npos)
					{
						if(fi1 < fi2)
						{
							int ilen = strlen("username=");
							username.assign(s.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
						}
					}

					username = HttpUtility::URLDecode(username);
					password = HttpUtility::URLDecode(password);

					// 向dbmgr重置账号
					Mercury::Bundle bundle;
					bundle.newMessage(DbmgrInterface::accountResetPassword);
					bundle << KBEngine::strutil::kbe_trim(username);
					bundle << KBEngine::strutil::kbe_trim(password);
					bundle << code;
					bundle.send(Loginapp::getSingleton().networkInterface(), dbmgrinfos->pChannel);
				}

				hellomessage = g_kbeSrvConfig.emailResetPasswordInfo_.backlink_hello_message;
			}
			else if(type == 3)
			{
				std::string username;

				std::string::size_type fi1 = s.find("username=");
				std::string::size_type fi2 = s.find(" HTTP/");
				
				if(fi1 != std::string::npos && fi2 != std::string::npos)
				{
					if(fi1 < fi2)
					{
						int ilen = strlen("username=");
						username.assign(s.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
					}
				}

				if(username.size() > 0)
				{
					username = HttpUtility::URLDecode(username);

					// 向dbmgr重置账号
					Mercury::Bundle bundle;
					bundle.newMessage(DbmgrInterface::accountBindMail);
					bundle << KBEngine::strutil::kbe_trim(username);
					bundle << code;
					bundle.send(Loginapp::getSingleton().networkInterface(), dbmgrinfos->pChannel);
				}

				hellomessage = g_kbeSrvConfig.emailBindInfo_.backlink_hello_message;
			}

			if(hellomessage.size() > 0 && client.state < 2)
			{
				KBEngine::strutil::kbe_replace(hellomessage, "${backlink}", (boost::format("http://%1%:%2%/%3%%4%") % 
					Loginapp::getSingleton().networkInterface().extaddr().ipAsString() %
					g_kbeSrvConfig.getLoginApp().http_cbport %
					keys %
					code).str());

				KBEngine::strutil::kbe_replace(hellomessage, "${code}", code);

				std::string response = (boost::format("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %1%\r\n\r\n%2%") % 
					hellomessage.size() % hellomessage).str();

				newclient->send(response.c_str(), response.size());
			}

			client.state = 2;
		}
		else
		{
			if(client.state != 2)
			{
				Loginapp::getSingleton().networkInterface().dispatcher().deregisterFileDescriptor(*newclient);
				clients_.erase(iter);
			}
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------
void HTTPCBHandler::onAccountActivated(std::string& code, bool success)
{
	std::map< int, CLIENT >::iterator iter = clients_.begin();
	for(; iter != clients_.end(); iter++)
	{
		if(iter->second.code == code)
		{
			if(!iter->second.endpoint->good())
				continue;
			
			std::string message;

			if(success)
				message = g_kbeSrvConfig.emailAtivationInfo_.backlink_success_message;
			else
				message = g_kbeSrvConfig.emailAtivationInfo_.backlink_fail_message;

			std::string response = (boost::format("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %1%\r\n\r\n%2%") % 
				message.size() % message).str();

			iter->second.endpoint->send(response.c_str(), response.size());
		}
	}
}

//-------------------------------------------------------------------------------------
void HTTPCBHandler::onAccountBindedEmail(std::string& code, bool success)
{
	std::map< int, CLIENT >::iterator iter = clients_.begin();
	for(; iter != clients_.end(); iter++)
	{
		if(iter->second.code == code)
		{
			if(!iter->second.endpoint->good())
				continue;
			
			std::string message;

			if(success)
				message = g_kbeSrvConfig.emailBindInfo_.backlink_success_message;
			else
				message = g_kbeSrvConfig.emailBindInfo_.backlink_fail_message;

			std::string response = (boost::format("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %1%\r\n\r\n%2%") % 
				message.size() % message).str();

			iter->second.endpoint->send(response.c_str(), response.size());
		}
	}
}

//-------------------------------------------------------------------------------------
void HTTPCBHandler::onAccountResetPassword(std::string& code, bool success)
{
	std::map< int, CLIENT >::iterator iter = clients_.begin();
	for(; iter != clients_.end(); iter++)
	{
		if(iter->second.code == code)
		{
			if(!iter->second.endpoint->good())
				continue;
			
			std::string message;

			if(success)
				message = g_kbeSrvConfig.emailResetPasswordInfo_.backlink_success_message;
			else
				message = g_kbeSrvConfig.emailResetPasswordInfo_.backlink_fail_message;

			std::string response = (boost::format("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %1%\r\n\r\n%2%") % 
				message.size() % message).str();

			iter->second.endpoint->send(response.c_str(), response.size());
		}
	}
}

//-------------------------------------------------------------------------------------
}
