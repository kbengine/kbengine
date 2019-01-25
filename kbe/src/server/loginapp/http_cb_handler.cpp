// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "loginapp.h"
#include "http_cb_handler.h"
#include "network/event_dispatcher.h"
#include "network/event_poller.h"
#include "network/endpoint.h"
#include "network/bundle.h"
#include "network/http_utility.h"
#include "helper/debug_helper.h"
#include "server/serverconfig.h"

#include "../../server/dbmgr/dbmgr_interface.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
HTTPCBHandler::HTTPCBHandler():
pEndPoint_(NULL),
clients_()
{
	pEndPoint_ = Network::EndPoint::createPoolObject(OBJECTPOOL_POINT);

	pEndPoint_->socket(SOCK_STREAM);

	if (!pEndPoint_->good())
	{
		ERROR_MSG("HTTPCBHandler::process: couldn't create a socket\n");
		return;
	}

	if (pEndPoint_->bind(htons(g_kbeSrvConfig.getLoginApp().http_cbport), 
		Loginapp::getSingleton().networkInterface().extTcpAddr().ip) == -1)
	{
		ERROR_MSG(fmt::format("HTTPCBHandler::bind({}): {}:{}\n",
			 kbe_strerror(), inet_ntoa((struct in_addr&)Loginapp::getSingleton().networkInterface().extTcpAddr().ip),
			g_kbeSrvConfig.getLoginApp().http_cbport));

		pEndPoint_->close();
		return;
	}

	if(pEndPoint_->listen() == -1)
	{
		ERROR_MSG(fmt::format("HTTPCBHandler::listeningSocket({}): {}:{}\n",
			 kbe_strerror(), inet_ntoa((struct in_addr&)Loginapp::getSingleton().networkInterface().extTcpAddr().ip),
			g_kbeSrvConfig.getLoginApp().http_cbport));

		pEndPoint_->close();
		return;
	}

	pEndPoint_->setnonblocking(true);

	Loginapp::getSingleton().networkInterface().dispatcher().registerReadFileDescriptor(*pEndPoint_, this);

	INFO_MSG(fmt::format("HTTPCBHandler::bind: {}:{}\n",
		inet_ntoa((struct in_addr&)Loginapp::getSingleton().networkInterface().extTcpAddr().ip),
		g_kbeSrvConfig.getLoginApp().http_cbport));
}

//-------------------------------------------------------------------------------------
HTTPCBHandler::~HTTPCBHandler()
{
	clients_.clear();
	Loginapp::getSingleton().networkInterface().dispatcher().deregisterReadFileDescriptor(*pEndPoint_);

	Network::EndPoint::reclaimPoolObject(pEndPoint_);
	pEndPoint_ = NULL;
}

//-------------------------------------------------------------------------------------
int HTTPCBHandler::handleInputNotification(int fd)
{
	if(fd == *pEndPoint_)
	{
		u_int16_t port;
		u_int32_t addr;

		Network::EndPoint* newclient = pEndPoint_->accept(&port, &addr);

		if(newclient == NULL)
		{
			ERROR_MSG(fmt::format("HTTPCBHandler::handleInputNotification: accept error:{}.\n", kbe_strerror()));
			return 0;
		}

		INFO_MSG(fmt::format("HTTPCBHandler:handleInputNotification: newclient = {}\n",
			newclient->c_str()));
		
		newclient->setnonblocking(true);
		CLIENT& client = clients_[*newclient];
		client.endpoint = KBEShared_ptr< Network::EndPoint >(newclient);
		client.state = 0;
		Loginapp::getSingleton().networkInterface().dispatcher().registerReadFileDescriptor(*newclient, this);
	}
	else
	{
		std::map< int, CLIENT >::iterator iter = clients_.find(fd);
		if(iter == clients_.end())
		{
			ERROR_MSG(fmt::format("HTTPCBHandler:handleInputNotification: fd({}) not found!\n",
				fd));

			return 0;
		}

		CLIENT& client = iter->second;
		Network::EndPoint* newclient = iter->second.endpoint.get();

		char buffer[1024];
		int len = newclient->recv(&buffer, 1024);

		if(len <= 0)
		{
			ERROR_MSG(fmt::format("HTTPCBHandler:handleInputNotification: recv error, newclient = {}, recv={}.\n",
				newclient->c_str(), len));
		
			if(len == 0)
			{
				Loginapp::getSingleton().networkInterface().dispatcher().deregisterReadFileDescriptor(*newclient);
				clients_.erase(iter);
			}

			return 0;
		}

		if(client.state == 1)
		{
			Loginapp::getSingleton().networkInterface().dispatcher().deregisterReadFileDescriptor(*newclient);
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
				iter->second.endpoint->send(response.c_str(), (int)response.size());
				Loginapp::getSingleton().networkInterface().dispatcher().deregisterReadFileDescriptor(*newclient);
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
			int ilen = (int)keys.size();
			code.assign(s.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
		}

		client.state = 1;
		
		code = KBEngine::strutil::kbe_trim(code);

		if(code.size() > 0)
		{
			INFO_MSG(fmt::format("HTTPCBHandler:handleInputNotification: code = {}\n",
				code.c_str()));

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
				// œÚdbmgrº§ªÓ’À∫≈
				Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
				(*pBundle).newMessage(DbmgrInterface::accountActivate);
				(*pBundle) << code;
				dbmgrinfos->pChannel->send(pBundle);

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
						int ilen = (int)strlen("password=");
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
							int ilen = (int)strlen("username=");
							username.assign(s.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
						}
					}

					username = Network::Http::URLDecode(username);
					password = Network::Http::URLDecode(password);

					// œÚdbmgr÷ÿ÷√’À∫≈
					Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
					(*pBundle).newMessage(DbmgrInterface::accountResetPassword);
					(*pBundle) << KBEngine::strutil::kbe_trim(username);
					(*pBundle) << KBEngine::strutil::kbe_trim(password);
					(*pBundle) << code;
					dbmgrinfos->pChannel->send(pBundle);
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
						int ilen = (int)strlen("username=");
						username.assign(s.c_str() + fi1 + ilen, fi2 - (fi1 + ilen));
					}
				}

				if(username.size() > 0)
				{
					username = Network::Http::URLDecode(username);

					// œÚdbmgr∞Û∂®’À∫≈’À∫≈
					Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
					(*pBundle).newMessage(DbmgrInterface::accountBindMail);
					(*pBundle) << KBEngine::strutil::kbe_trim(username);
					(*pBundle) << code;
					dbmgrinfos->pChannel->send(pBundle);
				}

				hellomessage = g_kbeSrvConfig.emailBindInfo_.backlink_hello_message;
			}

			if(hellomessage.size() > 0 && client.state < 2)
			{
				KBEngine::strutil::kbe_replace(hellomessage, "${backlink}", fmt::format("http://{}:{}/{}{}", 
					(strlen((const char*)&g_kbeSrvConfig.getLoginApp().externalAddress) > 0 ? 
					g_kbeSrvConfig.getLoginApp().externalAddress : 
					Loginapp::getSingleton().networkInterface().extTcpAddr().ipAsString()),
					g_kbeSrvConfig.getLoginApp().http_cbport,
					keys,
					code));

				KBEngine::strutil::kbe_replace(hellomessage, "${code}", code);

				std::string response = fmt::format("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: {}\r\n\r\n{}", 
					hellomessage.size(), hellomessage);

				newclient->send(response.c_str(), (int)response.size());
			}

			client.state = 2;
		}
		else
		{
			if(client.state != 2)
			{
				Loginapp::getSingleton().networkInterface().dispatcher().deregisterReadFileDescriptor(*newclient);
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
	for(; iter != clients_.end(); ++iter)
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

			std::string response = fmt::format("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: {}\r\n\r\n{}", 
				message.size(), message);

			iter->second.endpoint->send(response.c_str(), (int)response.size());
		}
	}
}

//-------------------------------------------------------------------------------------
void HTTPCBHandler::onAccountBindedEmail(std::string& code, bool success)
{
	std::map< int, CLIENT >::iterator iter = clients_.begin();
	for(; iter != clients_.end(); ++iter)
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

			std::string response = fmt::format("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: {}\r\n\r\n{}", 
				message.size(), message);

			iter->second.endpoint->send(response.c_str(), (int)response.size());
		}
	}
}

//-------------------------------------------------------------------------------------
void HTTPCBHandler::onAccountResetPassword(std::string& code, bool success)
{
	std::map< int, CLIENT >::iterator iter = clients_.begin();
	for(; iter != clients_.end(); ++iter)
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

			std::string response = fmt::format("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: {}\r\n\r\n{}", 
				message.size(), message);

			iter->second.endpoint->send(response.c_str(), (int)response.size());
		}
	}
}

//-------------------------------------------------------------------------------------
}
