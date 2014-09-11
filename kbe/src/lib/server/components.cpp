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


#include "components.hpp"
#include "componentbridge.hpp"
#include "helper/debug_helper.hpp"
#include "helper/sys_info.hpp"
#include "network/channel.hpp"	
#include "network/address.hpp"	
#include "network/bundle.hpp"	
#include "network/network_interface.hpp"
#include "client_lib/client_interface.hpp"
#include "server/serverconfig.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"
#include "../../server/tools/message_log/messagelog_interface.hpp"
#include "../../server/tools/bots/bots_interface.hpp"
#include "../../server/tools/billing_system/billingsystem_interface.hpp"

namespace KBEngine
{
int32 Components::ANY_UID = -1;

KBE_SINGLETON_INIT(Components);
Components _g_components;

//-------------------------------------------------------------------------------------
Components::Components():
_baseapps(),
_cellapps(),
_dbmgrs(),
_loginapps(),
_cellappmgrs(),
_baseappmgrs(),
_machines(),
_messagelogs(),
_billings(),
_bots(),
_consoles(),
_pNetworkInterface(NULL),
_globalOrderLog(),
_baseappGrouplOrderLog(),
_cellappGrouplOrderLog(),
_loginappGrouplOrderLog(),
_pHandler(NULL)
{
}

//-------------------------------------------------------------------------------------
Components::~Components()
{
}

//-------------------------------------------------------------------------------------
bool Components::checkComponents(int32 uid, COMPONENT_ID componentID)
{
	if(componentID <= 0)
		return true;

	int idx = 0;

	while(true)
	{
		COMPONENT_TYPE ct = ALL_COMPONENT_TYPES[idx++];
		if(ct == UNKNOWN_COMPONENT_TYPE)
			break;

		ComponentInfos* cinfos = findComponent(ct, uid, componentID);
		if(cinfos != NULL)
		{
			//ERROR_MSG(boost::format("Components::checkComponents: uid:%1%, componentType=%2%, componentID:%3% exist.\n") %
			//	uid % COMPONENT_NAME_EX(ct) % componentID);

			// KBE_ASSERT(false && "Components::checkComponents: componentID exist.\n");
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------		
void Components::addComponent(int32 uid, const char* username, 
			COMPONENT_TYPE componentType, COMPONENT_ID componentID, int8 globalorderid, int8 grouporderid,
			uint32 intaddr, uint16 intport, 
			uint32 extaddr, uint16 extport, std::string& extaddrEx, uint32 pid,
			float cpu, float mem, uint32 usedmem, uint64 extradata, uint64 extradata1, uint64 extradata2, uint64 extradata3,
			Mercury::Channel* pChannel)
{
	COMPONENTS& components = getComponents(componentType);

	if(!checkComponents(uid, componentID))
		return;

	ComponentInfos* cinfos = findComponent(componentType, uid, componentID);
	if(cinfos != NULL)
	{
		WARNING_MSG(boost::format("Components::addComponent[%1%]: uid:%2%, username:%3%, "
			"componentType:%4%, componentID:%5% is exist!\n") %
			COMPONENT_NAME_EX(componentType) % uid % username % (int32)componentType % componentID);
		return;
	}
	
	ComponentInfos componentInfos;

	componentInfos.pIntAddr.reset(new Mercury::Address(intaddr, intport));
	componentInfos.pExtAddr.reset(new Mercury::Address(extaddr, extport));

	if(extaddrEx.size() > 0)
		strncpy(componentInfos.externalAddressEx, extaddrEx.c_str(), MAX_NAME);
	
	componentInfos.uid = uid;
	componentInfos.cid = componentID;
	componentInfos.pChannel = pChannel;
	componentInfos.componentType = componentType;
	componentInfos.groupOrderid = 1;
	componentInfos.globalOrderid = 1;

	componentInfos.mem = mem;
	componentInfos.cpu = cpu;
	componentInfos.usedmem = usedmem;
	componentInfos.extradata = extradata;
	componentInfos.extradata1 = extradata1;
	componentInfos.extradata2 = extradata2;
	componentInfos.extradata3 = extradata3;
	componentInfos.pid = pid;

	if(pChannel)
		pChannel->componentID(componentID);

	strncpy(componentInfos.username, username, MAX_NAME);

	_globalOrderLog[uid]++;

	switch(componentType)
	{
	case BASEAPP_TYPE:
		_baseappGrouplOrderLog[uid]++;
		componentInfos.groupOrderid = _baseappGrouplOrderLog[uid];
		break;
	case CELLAPP_TYPE:
		_cellappGrouplOrderLog[uid]++;
		componentInfos.groupOrderid = _cellappGrouplOrderLog[uid];
		break;
	case LOGINAPP_TYPE:
		_loginappGrouplOrderLog[uid]++;
		componentInfos.groupOrderid = _loginappGrouplOrderLog[uid];
		break;
	default:
		break;
	};
	
	if(grouporderid > 0)
		componentInfos.groupOrderid = grouporderid;

	if(globalorderid > 0)
		componentInfos.globalOrderid = globalorderid;
	else
		componentInfos.globalOrderid = _globalOrderLog[uid];

	if(cinfos == NULL)
		components.push_back(componentInfos);
	else
		*cinfos = componentInfos;

	INFO_MSG(boost::format("Components::addComponent[%1%], uid=%2%, "
		"componentID=%3%, globalorderid=%4%, grouporderid=%5%, totalcount=%6%\n") %
			COMPONENT_NAME_EX(componentType) % 
			uid %
			componentID % 
			((int32)componentInfos.globalOrderid) %
			((int32)componentInfos.groupOrderid) %
			components.size());

	if(_pHandler)
		_pHandler->onAddComponent(&componentInfos);
}

//-------------------------------------------------------------------------------------		
void Components::delComponent(int32 uid, COMPONENT_TYPE componentType, 
							  COMPONENT_ID componentID, bool ignoreComponentID, bool shouldShowLog)
{
	COMPONENTS& components = getComponents(componentType);
	COMPONENTS::iterator iter = components.begin();
	for(; iter != components.end();)
	{
		if((uid < 0 || (*iter).uid == uid) && (ignoreComponentID == true || (*iter).cid == componentID))
		{
			INFO_MSG(boost::format("Components::delComponent[%1%] componentID=%2%, component:totalcount=%3%.\n") % 
				COMPONENT_NAME_EX(componentType) % componentID % components.size());

			ComponentInfos* componentInfos = &(*iter);

			//SAFE_RELEASE((*iter).pIntAddr);
			//SAFE_RELEASE((*iter).pExtAddr);
			//(*iter).pChannel->decRef();

			if(_pHandler)
				_pHandler->onRemoveComponent(componentInfos);

			iter = components.erase(iter);
			if(!ignoreComponentID)
				return;
		}
		else
			iter++;
	}

	if(shouldShowLog)
	{
		ERROR_MSG(boost::format("Components::delComponent::not found [%1%] component:totalcount:%2%\n") % 
			COMPONENT_NAME_EX(componentType) % components.size());
	}
}

//-------------------------------------------------------------------------------------		
void Components::removeComponentFromChannel(Mercury::Channel * pChannel)
{
	int ifind = 0;
	while(ALL_COMPONENT_TYPES[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		COMPONENT_TYPE componentType = ALL_COMPONENT_TYPES[ifind++];
		COMPONENTS& components = getComponents(componentType);
		COMPONENTS::iterator iter = components.begin();

		for(; iter != components.end();)
		{
			if((*iter).pChannel == pChannel)
			{
				//SAFE_RELEASE((*iter).pIntAddr);
				//SAFE_RELEASE((*iter).pExtAddr);
				// (*iter).pChannel->decRef();

				ERROR_MSG(boost::format("Components::removeComponentFromChannel: %1% : %2%.\n") %
					COMPONENT_NAME_EX(componentType) % (*iter).cid);

#if KBE_PLATFORM == PLATFORM_WIN32
				printf("[ERROR]: %s.\n", (boost::format("Components::removeComponentFromChannel: %1% : %2%.\n") %
					COMPONENT_NAME_EX(componentType) % (*iter).cid).str().c_str());
#endif

				ComponentInfos* componentInfos = &(*iter);

				if(_pHandler)
					_pHandler->onRemoveComponent(componentInfos);

				iter = components.erase(iter);
				return;
			}
			else
				iter++;
		}
	}

	// KBE_ASSERT(false && "channel is not found!\n");
}

//-------------------------------------------------------------------------------------		
int Components::connectComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID)
{
	Components::ComponentInfos* pComponentInfos = findComponent(componentType, uid, componentID);
	KBE_ASSERT(pComponentInfos != NULL);

	Mercury::EndPoint * pEndpoint = new Mercury::EndPoint;
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("Components::connectComponent: couldn't create a socket\n");
		delete pEndpoint;
		return -1;
	}

	pEndpoint->addr(*pComponentInfos->pIntAddr);
	int ret = pEndpoint->connect(pComponentInfos->pIntAddr->port, pComponentInfos->pIntAddr->ip);

	if(ret == 0)
	{
		pComponentInfos->pChannel = new Mercury::Channel(*_pNetworkInterface, pEndpoint, Mercury::Channel::INTERNAL);
		pComponentInfos->pChannel->componentID(componentID);
		if(!_pNetworkInterface->registerChannel(pComponentInfos->pChannel))
		{
			ERROR_MSG(boost::format("Components::connectComponent: registerChannel(%1%) is failed!\n") %
				pComponentInfos->pChannel->c_str());

			pComponentInfos->pChannel->destroy();
			pComponentInfos->pChannel = NULL;
			return -1;
		}
		else
		{
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			if(componentType == BASEAPPMGR_TYPE)
			{
				(*pBundle).newMessage(BaseappmgrInterface::onRegisterNewApp);
				
				BaseappmgrInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == CELLAPPMGR_TYPE)
			{
				(*pBundle).newMessage(CellappmgrInterface::onRegisterNewApp);
				
				CellappmgrInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == CELLAPP_TYPE)
			{
				(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
				
				CellappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					g_componentGlobalOrder, g_componentGroupOrder,
						_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == BASEAPP_TYPE)
			{
				(*pBundle).newMessage(BaseappInterface::onRegisterNewApp);
				
				BaseappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == DBMGR_TYPE)
			{
				(*pBundle).newMessage(DbmgrInterface::onRegisterNewApp);
				
				DbmgrInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == MESSAGELOG_TYPE)
			{
				(*pBundle).newMessage(MessagelogInterface::onRegisterNewApp);
				
				MessagelogInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else
			{
				KBE_ASSERT(false && "invalid componentType.\n");
			}

			(*pBundle).send(*_pNetworkInterface, pComponentInfos->pChannel);
			Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		}
	}
	else
	{
		ERROR_MSG(boost::format("Components::connectComponent: connect(%1%) is failed! %2%.\n") %
			pComponentInfos->pIntAddr->c_str() % kbe_strerror());

		return -1;
	}

	return ret;
}

//-------------------------------------------------------------------------------------		
void Components::clear(int32 uid, bool shouldShowLog)
{
	delComponent(uid, DBMGR_TYPE, uid, true, shouldShowLog);
	delComponent(uid, BASEAPPMGR_TYPE, uid, true, shouldShowLog);
	delComponent(uid, CELLAPPMGR_TYPE, uid, true, shouldShowLog);
	delComponent(uid, CELLAPP_TYPE, uid, true, shouldShowLog);
	delComponent(uid, BASEAPP_TYPE, uid, true, shouldShowLog);
	delComponent(uid, LOGINAPP_TYPE, uid, true, shouldShowLog);
	//delComponent(uid, MESSAGELOG_TYPE, uid, true, shouldShowLog);
}

//-------------------------------------------------------------------------------------		
Components::COMPONENTS& Components::getComponents(COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case DBMGR_TYPE:
		return _dbmgrs;
	case LOGINAPP_TYPE:
		return _loginapps;
	case BASEAPPMGR_TYPE:
		return _baseappmgrs;
	case CELLAPPMGR_TYPE:
		return _cellappmgrs;
	case CELLAPP_TYPE:
		return _cellapps;
	case BASEAPP_TYPE:
		return _baseapps;
	case MACHINE_TYPE:
		return _machines;
	case MESSAGELOG_TYPE:
		return _messagelogs;			
	case BILLING_TYPE:
		return _billings;	
	case BOTS_TYPE:
		return _bots;	
	default:
		break;
	};

	return _consoles;
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::findComponent(COMPONENT_TYPE componentType, int32 uid,
																			COMPONENT_ID componentID)
{
	COMPONENTS& components = getComponents(componentType);
	COMPONENTS::iterator iter = components.begin();
	for(; iter != components.end(); iter++)
	{
		if((*iter).uid == uid && (componentID == 0 || (*iter).cid == componentID))
			return &(*iter);
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	COMPONENTS& components = getComponents(componentType);
	COMPONENTS::iterator iter = components.begin();
	for(; iter != components.end(); iter++)
	{
		if(componentID == 0 || (*iter).cid == componentID)
			return &(*iter);
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::findComponent(COMPONENT_ID componentID)
{
	int idx = 0;
	int32 uid = getUserUID();

	while(true)
	{
		COMPONENT_TYPE ct = ALL_COMPONENT_TYPES[idx++];
		if(ct == UNKNOWN_COMPONENT_TYPE)
			break;

		ComponentInfos* cinfos = findComponent(ct, uid, componentID);
		if(cinfos != NULL)
		{
			return cinfos;
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::findComponent(Mercury::Channel * pChannel)
{
	int ifind = 0;
	while(ALL_COMPONENT_TYPES[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		COMPONENT_TYPE componentType = ALL_COMPONENT_TYPES[ifind++];
		COMPONENTS& components = getComponents(componentType);
		COMPONENTS::iterator iter = components.begin();

		for(; iter != components.end(); iter++)
		{
			if((*iter).pChannel == pChannel)
			{
				return &(*iter);
			}
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
bool Components::checkComponentUsable(const Components::ComponentInfos* info)
{
	// 不对其他machine做处理
	if(info->componentType == MACHINE_TYPE)
	{
		return true;
	}

	bool islocal = _pNetworkInterface->intaddr().ip == info->pIntAddr->ip ||
			_pNetworkInterface->extaddr().ip == info->pIntAddr->ip;

	// 如果是本机应用则判断是否还在运行中
	if(islocal && info->pid > 0)
	{
		SystemInfo::PROCESS_INFOS sysinfos = SystemInfo::getSingleton().getProcessInfo(info->pid);
		if(sysinfos.error)
		{
			WARNING_MSG(boost::format("Components::checkComponentUsable: not found pid(%1%)\n") % info->pid);
			//return false;
		}
		else
		{
			Components::ComponentInfos* winfo = findComponent(info->cid);
			if(winfo)
			{
				winfo->cpu = sysinfos.cpu;
				winfo->usedmem = (uint32)sysinfos.memused;

				winfo->mem = float((winfo->usedmem * 1.0 / SystemInfo::getSingleton().totalmem()) * 100.0);
			}
		}
	}

	Mercury::EndPoint epListen;
	epListen.socket(SOCK_STREAM);
	if (!epListen.good())
	{
		ERROR_MSG("Components::checkComponentUsable: couldn't create a socket\n");
		return true;
	}
	
	epListen.setnonblocking(true);

	while(true)
	{
		fd_set	frds, fwds;
		struct timeval tv = { 0, 300000 }; // 100ms

		FD_ZERO( &frds );
		FD_ZERO( &fwds );
		FD_SET((int)epListen, &frds);
		FD_SET((int)epListen, &fwds);

		if(epListen.connect(info->pIntAddr->port, info->pIntAddr->ip) == -1)
		{
			int selgot = select(epListen+1, &frds, &fwds, NULL, &tv);
			if(selgot > 0)
			{
				break;
			}

			ERROR_MSG(boost::format("Components::checkComponentUsable: couldn't connect to:%1%\n") % info->pIntAddr->c_str());
			return false;
		}
	}
	
	epListen.setnodelay(true);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	// 由于COMMON_MERCURY_MESSAGE不包含client， 如果是bots， 我们需要单独处理
	if(info->componentType != BOTS_TYPE)
	{
		COMMON_MERCURY_MESSAGE(info->componentType, (*pBundle), lookApp);
	}
	else
	{
		(*pBundle).newMessage(BotsInterface::lookApp);
	}

	epListen.send(pBundle->pCurrPacket()->data(), pBundle->pCurrPacket()->wpos());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	fd_set	fds;
	struct timeval tv = { 0, 300000 }; // 100ms

	FD_ZERO( &fds );
	FD_SET((int)epListen, &fds);

	int selgot = select(epListen+1, &fds, NULL, NULL, &tv);
	if(selgot == 0)
	{
		return true;	// 超时可能对方繁忙
	}
	else if(selgot == -1)
	{
		return true;
	}
	else
	{
		COMPONENT_TYPE ctype;
		COMPONENT_ID cid;
		int8 istate = 0;
		ArraySize entitySize = 0, cellSize = 0;
		int32 clientsSize = 0, proxicesSize = 0;
		uint32 telnet_port = 0;

		Mercury::TCPPacket packet;
		packet.resize(255);
		int recvsize = sizeof(ctype) + sizeof(cid) + sizeof(istate);

		if(info->componentType == CELLAPP_TYPE)
		{
			recvsize += sizeof(entitySize) + sizeof(cellSize) + sizeof(telnet_port);
		}

		if(info->componentType == BASEAPP_TYPE)
		{
			recvsize += sizeof(entitySize) + sizeof(clientsSize) + sizeof(proxicesSize) + sizeof(telnet_port);
		}

		int len = epListen.recv(packet.data(), recvsize);
		packet.wpos(len);
		
		if(recvsize != len)
		{
			ERROR_MSG(boost::format("Components::checkComponentUsable: packet invalid(recvsize(%1%) != ctype_cid_len(%2%).\n") 
				% len % recvsize);
			
			if(len == 0)
				return false;

			return true;
		}

		packet >> ctype >> cid >> istate;
		
		if(ctype == CELLAPP_TYPE)
		{
			packet >> entitySize >> cellSize >> telnet_port;
		}

		if(ctype == BASEAPP_TYPE)
		{
			packet >> entitySize >> clientsSize >> proxicesSize >> telnet_port;
		}

		if(ctype != info->componentType || cid != info->cid)
		{
			ERROR_MSG(boost::format("Components::checkComponentUsable: invalid component(ctype=%1%, cid=%2%).\n") %
				ctype % cid);

			return false;
		}

		Components::ComponentInfos* winfo = findComponent(info->cid);
		if(winfo)
		{
			winfo->shutdownState = istate;

			if(ctype == CELLAPP_TYPE)
			{
				winfo->extradata = entitySize;
				winfo->extradata1 = cellSize;
				winfo->extradata3 = telnet_port;
			}
			
			if(ctype == BASEAPP_TYPE)
			{
				winfo->extradata = entitySize;
				winfo->extradata1 = clientsSize;
				winfo->extradata2 = proxicesSize;
				winfo->extradata3 = telnet_port;
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::getBaseappmgr()
{
	return findComponent(BASEAPPMGR_TYPE, getUserUID(), 0);
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::getCellappmgr()
{
	return findComponent(CELLAPPMGR_TYPE, getUserUID(), 0);
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::getDbmgr()
{
	return findComponent(DBMGR_TYPE, getUserUID(), 0);
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::getMessagelog()
{
	return findComponent(MESSAGELOG_TYPE, getUserUID(), 0);
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::getBillings()
{
	return findComponent(BILLING_TYPE, getUserUID(), 0);
}

//-------------------------------------------------------------------------------------		
Mercury::Channel* Components::getBaseappmgrChannel()
{
	Components::ComponentInfos* cinfo = getBaseappmgr();
	if(cinfo == NULL)
		 return NULL;

	return cinfo->pChannel;
}

//-------------------------------------------------------------------------------------		
Mercury::Channel* Components::getCellappmgrChannel()
{
	Components::ComponentInfos* cinfo = getCellappmgr();
	if(cinfo == NULL)
		 return NULL;

	return cinfo->pChannel;
}

//-------------------------------------------------------------------------------------		
Mercury::Channel* Components::getDbmgrChannel()
{
	Components::ComponentInfos* cinfo = getDbmgr();
	if(cinfo == NULL)
		 return NULL;

	return cinfo->pChannel;
}

//-------------------------------------------------------------------------------------		
Mercury::Channel* Components::getMessagelogChannel()
{
	Components::ComponentInfos* cinfo = getMessagelog();
	if(cinfo == NULL)
		 return NULL;

	return cinfo->pChannel;
}

//-------------------------------------------------------------------------------------		
	
}
