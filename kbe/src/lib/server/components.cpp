/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "components.h"
#include "helper/debug_helper.h"
#include "helper/sys_info.h"
#include "network/channel.h"	
#include "network/address.h"	
#include "network/bundle.h"	
#include "network/udp_packet.h"
#include "network/tcp_packet.h"
#include "network/bundle_broadcast.h"
#include "network/network_interface.h"
#include "client_lib/client_interface.h"
#include "server/serverconfig.h"

#include "../../server/baseappmgr/baseappmgr_interface.h"
#include "../../server/cellappmgr/cellappmgr_interface.h"
#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"
#include "../../server/loginapp/loginapp_interface.h"
#include "../../server/tools/logger/logger_interface.h"
#include "../../server/tools/bots/bots_interface.h"
#include "../../server/tools/interfaces/interfaces_interface.h"

#include "../../server/machine/machine_interface.h"

namespace KBEngine
{
int32 Components::ANY_UID = -1;

KBE_SINGLETON_INIT(Components);
Components _g_components;

//-------------------------------------------------------------------------------------
Components::Components():
Task(),
_baseapps(),
_cellapps(),
_dbmgrs(),
_loginapps(),
_cellappmgrs(),
_baseappmgrs(),
_machines(),
_loggers(),
_interfaceses(),
_bots(),
_consoles(),
_pNetworkInterface(NULL),
_globalOrderLog(),
_baseappGrouplOrderLog(),
_cellappGrouplOrderLog(),
_loginappGrouplOrderLog(),
_pHandler(NULL),
componentType_(UNKNOWN_COMPONENT_TYPE),
componentID_(0),
state_(0),
findIdx_(0),
extraData1_(0),
extraData2_(0),
extraData3_(0),
extraData4_(0)
{
}

//-------------------------------------------------------------------------------------
Components::~Components()
{
}

//-------------------------------------------------------------------------------------
void Components::initialize(Network::NetworkInterface * pNetworkInterface, COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{ 
	KBE_ASSERT(pNetworkInterface != NULL); 
	_pNetworkInterface = pNetworkInterface; 

	componentType_ = componentType;
	componentID_ = componentID;

	for(uint8 i=0; i<8; ++i)
		findComponentTypes_[i] = UNKNOWN_COMPONENT_TYPE;

	switch(componentType_)
	{
	case CELLAPP_TYPE:
		findComponentTypes_[0] = LOGGER_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = CELLAPPMGR_TYPE;
		findComponentTypes_[3] = BASEAPPMGR_TYPE;
		break;
	case BASEAPP_TYPE:
		findComponentTypes_[0] = LOGGER_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = BASEAPPMGR_TYPE;
		findComponentTypes_[3] = CELLAPPMGR_TYPE;
		break;
	case BASEAPPMGR_TYPE:
		findComponentTypes_[0] = LOGGER_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = CELLAPPMGR_TYPE;
		break;
	case CELLAPPMGR_TYPE:
		findComponentTypes_[0] = LOGGER_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = BASEAPPMGR_TYPE;
		break;
	case LOGINAPP_TYPE:
		findComponentTypes_[0] = LOGGER_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = BASEAPPMGR_TYPE;
		break;
	case DBMGR_TYPE:
		findComponentTypes_[0] = LOGGER_TYPE;
		break;
	default:
		if(componentType_ != LOGGER_TYPE && 
			componentType_ != MACHINE_TYPE && 
			componentType_ != INTERFACES_TYPE)
			findComponentTypes_[0] = LOGGER_TYPE;
		break;
	};
}

//-------------------------------------------------------------------------------------
void Components::finalise()
{
	clear(0, false);
}

//-------------------------------------------------------------------------------------
bool Components::checkComponents(int32 uid, COMPONENT_ID componentID, uint32 pid)
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
			if(cinfos->componentType != MACHINE_TYPE && cinfos->pid != 0 /* ����0ͨ����Ԥ�裬 ������������Ȳ����Ƚ� */ && pid != cinfos->pid)
			{
				ERROR_MSG(fmt::format("Components::checkComponents: uid:{}, componentType={}, componentID:{} exist.\n",
					uid, COMPONENT_NAME_EX(ct), componentID));

				KBE_ASSERT(false && "Components::checkComponents: componentID exist.\n");
			}
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------		
void Components::addComponent(int32 uid, const char* username, 
			COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderid, COMPONENT_ORDER grouporderid,
			uint32 intaddr, uint16 intport, 
			uint32 extaddr, uint16 extport, std::string& extaddrEx, uint32 pid,
			float cpu, float mem, uint32 usedmem, uint64 extradata, uint64 extradata1, uint64 extradata2, uint64 extradata3,
			Network::Channel* pChannel)
{
	COMPONENTS& components = getComponents(componentType);

	if(!checkComponents(uid, componentID, pid))
		return;

	ComponentInfos* cinfos = findComponent(componentType, uid, componentID);
	if(cinfos != NULL)
	{
		WARNING_MSG(fmt::format("Components::addComponent[{}]: uid:{}, username:{}, "
			"componentType:{}, componentID:{} is exist!\n",
			COMPONENT_NAME_EX(componentType), uid, username, (int32)componentType, componentID));
		return;
	}
	
	ComponentInfos componentInfos;

	componentInfos.pIntAddr.reset(new Network::Address(intaddr, intport));
	componentInfos.pExtAddr.reset(new Network::Address(extaddr, extport));

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

	INFO_MSG(fmt::format("Components::addComponent[{}], uid={}, "
		"componentID={}, globalorderid={}, grouporderid={}, totalcount={}\n",
			COMPONENT_NAME_EX(componentType), 
			uid,
			componentID, 
			((int32)componentInfos.globalOrderid),
			((int32)componentInfos.groupOrderid),
			components.size()));
	
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
			INFO_MSG(fmt::format("Components::delComponent[{}] componentID={}, component:totalcount={}.\n", 
				COMPONENT_NAME_EX(componentType), componentID, components.size()));

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
		ERROR_MSG(fmt::format("Components::delComponent::not found [{}] component:totalcount:{}\n", 
			COMPONENT_NAME_EX(componentType), components.size()));
	}
}

//-------------------------------------------------------------------------------------		
void Components::removeComponentByChannel(Network::Channel * pChannel, bool isShutingdown)
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

				if(!isShutingdown)
				{
					ERROR_MSG(fmt::format("Components::removeComponentByChannel: {} : {}, Abnormal exit.\n",
						COMPONENT_NAME_EX(componentType), (*iter).cid));

#if KBE_PLATFORM == PLATFORM_WIN32
					printf("[ERROR]: %s.\n", (fmt::format("Components::removeComponentByChannel: {} : {}, Abnormal exit!\n",
						COMPONENT_NAME_EX(componentType), (*iter).cid)).c_str());
#endif
				}
				else
				{
					INFO_MSG(fmt::format("Components::removeComponentByChannel: {} : {}, Normal exit!\n",
						COMPONENT_NAME_EX(componentType), (*iter).cid));
				}

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
int Components::connectComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID, bool printlog)
{
	Components::ComponentInfos* pComponentInfos = findComponent(componentType, uid, componentID);
	KBE_ASSERT(pComponentInfos != NULL);

	Network::EndPoint * pEndpoint = Network::EndPoint::ObjPool().createObject();
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		if (printlog)
		{
			ERROR_MSG("Components::connectComponent: couldn't create a socket\n");
		}

		Network::EndPoint::ObjPool().reclaimObject(pEndpoint);
		return -1;
	}

	pEndpoint->addr(*pComponentInfos->pIntAddr);
	int ret = pEndpoint->connect(pComponentInfos->pIntAddr->port, pComponentInfos->pIntAddr->ip);

	if(ret == 0)
	{
		Network::Channel* pChannel = Network::Channel::ObjPool().createObject();
		bool ret = pChannel->initialize(*_pNetworkInterface, pEndpoint, Network::Channel::INTERNAL);
		if(!ret)
		{
			if (printlog)
			{
				ERROR_MSG(fmt::format("Components::connectComponent: initialize({}) is failed!\n",
					pChannel->c_str()));
			}

			pChannel->destroy();
			Network::Channel::ObjPool().reclaimObject(pChannel);
			return -1;
		}

		pComponentInfos->pChannel = pChannel;
		pComponentInfos->pChannel->componentID(componentID);
		if(!_pNetworkInterface->registerChannel(pComponentInfos->pChannel))
		{
			if (printlog)
			{
				ERROR_MSG(fmt::format("Components::connectComponent: registerChannel({}) is failed!\n",
					pComponentInfos->pChannel->c_str()));
			}

			pComponentInfos->pChannel->destroy();
			Network::Channel::ObjPool().reclaimObject(pComponentInfos->pChannel);

			// ��ʱ����ǿ���ͷ��ڴ棬destroy���Ѿ����������
			// SAFE_RELEASE(pComponentInfos->pChannel);
			pComponentInfos->pChannel = NULL;
			return -1;
		}
		else
		{
			Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
			if(componentType == BASEAPPMGR_TYPE)
			{
				(*pBundle).newMessage(BaseappmgrInterface::onRegisterNewApp);
				
				BaseappmgrInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					componentType_, componentID_, 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == CELLAPPMGR_TYPE)
			{
				(*pBundle).newMessage(CellappmgrInterface::onRegisterNewApp);
				
				CellappmgrInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					componentType_, componentID_, 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == CELLAPP_TYPE)
			{
				(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
				
				CellappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					componentType_, componentID_, 
					g_componentGlobalOrder, g_componentGroupOrder,
						_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == BASEAPP_TYPE)
			{
				(*pBundle).newMessage(BaseappInterface::onRegisterNewApp);
				
				BaseappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					componentType_, componentID_, 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == DBMGR_TYPE)
			{
				(*pBundle).newMessage(DbmgrInterface::onRegisterNewApp);
				
				DbmgrInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					componentType_, componentID_, 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else if(componentType == LOGGER_TYPE)
			{
				(*pBundle).newMessage(LoggerInterface::onRegisterNewApp);
				
				LoggerInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
					componentType_, componentID_, 
					g_componentGlobalOrder, g_componentGroupOrder,
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
			}
			else
			{
				KBE_ASSERT(false && "invalid componentType.\n");
			}

			pComponentInfos->pChannel->send(pBundle);
		}
	}
	else
	{
		if (printlog)
		{
			ERROR_MSG(fmt::format("Components::connectComponent: connect({}) is failed! {}.\n",
				pComponentInfos->pIntAddr->c_str(), kbe_strerror()));
		}

		Network::EndPoint::ObjPool().reclaimObject(pEndpoint);
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
	//delComponent(uid, LOGGER_TYPE, uid, true, shouldShowLog);
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
	case LOGGER_TYPE:
		return _loggers;			
	case INTERFACES_TYPE:
		return _interfaceses;	
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
	for(; iter != components.end(); ++iter)
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
	for(; iter != components.end(); ++iter)
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
Components::ComponentInfos* Components::findComponent(Network::Channel * pChannel)
{
	int ifind = 0;

	while(ALL_COMPONENT_TYPES[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		COMPONENT_TYPE componentType = ALL_COMPONENT_TYPES[ifind++];
		COMPONENTS& components = getComponents(componentType);
		COMPONENTS::iterator iter = components.begin();

		for(; iter != components.end(); ++iter)
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
Components::ComponentInfos* Components::findComponent(Network::Address* pAddress)
{
	int ifind = 0;

	while(ALL_COMPONENT_TYPES[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		COMPONENT_TYPE componentType = ALL_COMPONENT_TYPES[ifind++];
		COMPONENTS& components = getComponents(componentType);
		COMPONENTS::iterator iter = components.begin();

		for(; iter != components.end(); ++iter)
		{
			if((*iter).pChannel && (*iter).pChannel->addr() == *pAddress)
			{
				return &(*iter);
			}
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::findLocalComponent(uint32 pid)
{
	int ifind = 0;
	while(ALL_COMPONENT_TYPES[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		COMPONENT_TYPE componentType = ALL_COMPONENT_TYPES[ifind++];
		COMPONENTS& components = getComponents(componentType);
		COMPONENTS::iterator iter = components.begin();

		for(; iter != components.end(); ++iter)
		{
			if(isLocalComponent(&(*iter)) && (*iter).pid == pid)
			{
				return &(*iter);
			}
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
bool Components::isLocalComponent(const Components::ComponentInfos* info)
{
	return _pNetworkInterface->intaddr().ip == info->pIntAddr->ip ||
			_pNetworkInterface->extaddr().ip == info->pIntAddr->ip;
}

//-------------------------------------------------------------------------------------		
const Components::ComponentInfos* Components::lookupLocalComponentRunning(uint32 pid)
{
	if(pid > 0)
	{
		SystemInfo::PROCESS_INFOS sysinfos = SystemInfo::getSingleton().getProcessInfo(pid);
		if(sysinfos.error)
		{
			return NULL;
		}
		else
		{
			Components::ComponentInfos* winfo = findLocalComponent(pid);

			if(winfo)
			{
				winfo->cpu = sysinfos.cpu;
				winfo->usedmem = (uint32)sysinfos.memused;

				winfo->mem = float((winfo->usedmem * 1.0 / SystemInfo::getSingleton().totalmem()) * 100.0);
			}

			return winfo;
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
bool Components::updateComponentInfos(const Components::ComponentInfos* info)
{
	// ��������machine������
	if(info->componentType == MACHINE_TYPE)
	{
		return true;
	}

	Network::EndPoint epListen;
	epListen.socket(SOCK_STREAM);
	if (!epListen.good())
	{
		ERROR_MSG("Components::updateComponentInfos: couldn't create a socket\n");
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

			WARNING_MSG(fmt::format("Components::updateComponentInfos: couldn't connect to:{}\n", 
				info->pIntAddr->c_str()));

			return false;
		}
	}
	
	epListen.setnodelay(true);

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();

	// ����COMMON_NETWORK_MESSAGE������client�� �����bots�� ������Ҫ��������
	if(info->componentType != BOTS_TYPE)
	{
		COMMON_NETWORK_MESSAGE(info->componentType, (*pBundle), lookApp);
	}
	else
	{
		(*pBundle).newMessage(BotsInterface::lookApp);
	}

	epListen.send(pBundle->pCurrPacket()->data(), pBundle->pCurrPacket()->wpos());
	Network::Bundle::ObjPool().reclaimObject(pBundle);

	fd_set	fds;
	struct timeval tv = { 0, 300000 }; // 100ms

	FD_ZERO( &fds );
	FD_SET((int)epListen, &fds);

	int selgot = select(epListen+1, &fds, NULL, NULL, &tv);
	if(selgot == 0)
	{
		// ��ʱ, ���ܶԷ���æ
		return true;	
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

		Network::TCPPacket packet;
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
			WARNING_MSG(fmt::format("Components::updateComponentInfos: packet invalid(recvsize({}) != ctype_cid_len({}).\n" 
				, len, recvsize));
			
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
			WARNING_MSG(fmt::format("Components::updateComponentInfos: invalid component(ctype={}, cid={}).\n",
				ctype, cid));

			return false;
		}

		Components::ComponentInfos* winfo = findComponent(info->cid);
		if(winfo)
		{
			winfo->state = (COMPONENT_STATE)istate;

			if(ctype == CELLAPP_TYPE)
			{
				winfo->extradata = entitySize;
				winfo->extradata1 = cellSize;
				winfo->extradata3 = telnet_port;
			}
			else if(ctype == BASEAPP_TYPE)
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
Components::ComponentInfos* Components::getLogger()
{
	return findComponent(LOGGER_TYPE, getUserUID(), 0);
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::getInterfaceses()
{
	return findComponent(INTERFACES_TYPE, getUserUID(), 0);
}

//-------------------------------------------------------------------------------------		
Network::Channel* Components::getBaseappmgrChannel()
{
	Components::ComponentInfos* cinfo = getBaseappmgr();
	if(cinfo == NULL)
		 return NULL;

	return cinfo->pChannel;
}

//-------------------------------------------------------------------------------------		
Network::Channel* Components::getCellappmgrChannel()
{
	Components::ComponentInfos* cinfo = getCellappmgr();
	if(cinfo == NULL)
		 return NULL;

	return cinfo->pChannel;
}

//-------------------------------------------------------------------------------------		
Network::Channel* Components::getDbmgrChannel()
{
	Components::ComponentInfos* cinfo = getDbmgr();
	if(cinfo == NULL)
		 return NULL;

	return cinfo->pChannel;
}

//-------------------------------------------------------------------------------------		
Network::Channel* Components::getLoggerChannel()
{
	Components::ComponentInfos* cinfo = getLogger();
	if(cinfo == NULL)
		 return NULL;

	return cinfo->pChannel;
}

//-------------------------------------------------------------------------------------	
size_t Components::getGameSrvComponentsSize()
{
	COMPONENTS	_baseapps;
	COMPONENTS	_cellapps;
	COMPONENTS	_dbmgrs;
	COMPONENTS	_loginapps;
	COMPONENTS	_cellappmgrs;
	COMPONENTS	_baseappmgrs;

	return _baseapps.size() + _cellapps.size() + _dbmgrs.size() + 
		_loginapps.size() + _cellappmgrs.size() + _baseappmgrs.size();
}

//-------------------------------------------------------------------------------------
Network::EventDispatcher & Components::dispatcher()
{
	return pNetworkInterface()->dispatcher();
}

//-------------------------------------------------------------------------------------
void Components::onChannelDeregister(Network::Channel * pChannel, bool isShutingdown)
{
	removeComponentByChannel(pChannel, isShutingdown);
}

//-------------------------------------------------------------------------------------
bool Components::findLogger()
{
	if(g_componentType == LOGGER_TYPE || g_componentType == MACHINE_TYPE ||
		componentType_ == INTERFACES_TYPE)
	{
		return true;
	}
	
	int i = 0;
	
	while(i++ < 1/*���Logger��������Ϸ����ͬʱ�����������趨�Ĳ��Ҵ���Խ�࣬
		�ҵ�Logger�ĸ���Խ�󣬵�ǰֻ�趨����һ�Σ��ٶ��û��Ѿ���ǰ������Logger����*/)
	{
		srand(KBEngine::getSystemTime());
		uint16 nport = KBE_PORT_START + (rand() % 1000);
			
		Network::BundleBroadcast bhandler(*pNetworkInterface(), nport);
		if(!bhandler.good())
		{
			continue;
		}

		bhandler.itry(0);
		if(bhandler.pCurrPacket() != NULL)
		{
			bhandler.pCurrPacket()->resetPacket();
		}
			
		COMPONENT_TYPE findComponentType = LOGGER_TYPE;
		bhandler.newMessage(MachineInterface::onFindInterfaceAddr);
		MachineInterface::onFindInterfaceAddrArgs7::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
			g_componentType, g_componentID, findComponentType, pNetworkInterface()->intaddr().ip, bhandler.epListen().addr().port);

		if(!bhandler.broadcast())
		{
			//ERROR_MSG("Components::findLogger: broadcast error!\n");
			continue;
		}

		int32 timeout = 1500000;
		MachineInterface::onBroadcastInterfaceArgs24 args;

RESTART_RECV:

		if(bhandler.receive(&args, 0, timeout, false))
		{
			bool isContinue = false;
			timeout = 1000000;

			do
			{
				if(isContinue)
				{
					try
					{
						args.createFromStream(*bhandler.pCurrPacket());
					}catch(MemoryStreamException &)
					{
						break;
					}
				}
				
				if(args.componentIDEx != g_componentID)
				{
					//WARNING_MSG(fmt::format("Components::findLogger: msg.componentID {} != {}.\n", 
					//	args.componentIDEx, g_componentID));
					
					args.componentIDEx = 0;
					goto RESTART_RECV;
				}

				// ����Ҳ���
				if(args.componentType == UNKNOWN_COMPONENT_TYPE)
				{
					isContinue = true;
					continue;
				}

				INFO_MSG(fmt::format("Components::findLogger: found {}, addr:{}:{}\n",
					COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType),
					inet_ntoa((struct in_addr&)args.intaddr),
					ntohs(args.intport)));

				Components::getSingleton().addComponent(args.uid, args.username.c_str(), 
					(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.globalorderid, args.grouporderid, 
					args.intaddr, args.intport, args.extaddr, args.extport, args.extaddrEx, args.pid, args.cpu, args.mem, 
					args.usedmem, args.extradata, args.extradata1, args.extradata2, 123);

				isContinue = true;
			}while(bhandler.pCurrPacket()->length() > 0);

			// ��ֹ���յ������ݲ�����Ҫ������
			if(findComponentType == args.componentType)
			{
				for(int iconn=0; iconn<5; iconn++)
				{
					if(connectComponent(static_cast<COMPONENT_TYPE>(findComponentType), getUserUID(), 0, false) != 0)
					{
						//ERROR_MSG(fmt::format("Components::findLogger: register self to {} is error!\n",
						//COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));
						//dispatcher().breakProcessing();
						KBEngine::sleep(200);
					}
					else
					{
						//findComponentTypes_[0] = -1;
						for(size_t ic=1; ic<sizeof(findComponentTypes_) - 1; ++ic)
						{
							findComponentTypes_[ic - 1] = findComponentTypes_[ic];
						}

						return true;
					}
				}
			}
		}
		else
		{
			// �������ݳ�ʱ��
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool Components::findComponents()
{
	if(state_ == 1)
	{
		srand(KBEngine::getSystemTime());
		uint16 nport = KBE_PORT_START + (rand() % 1000);

		while(findComponentTypes_[findIdx_] != UNKNOWN_COMPONENT_TYPE)
		{
			if(dispatcher().hasBreakProcessing() || dispatcher().waitingBreakProcessing())
				return false;

			COMPONENT_TYPE findComponentType = (COMPONENT_TYPE)findComponentTypes_[findIdx_];
			static int count = 0;

			if(count <= 15)
			{
				INFO_MSG(fmt::format("Components::findComponents: find {}({})...\n",
					COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType), ++count));
			}
			else
			{
				std::string s = fmt::format("Components::findComponents: find {}({})...\n",
					COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType), ++count);
				WARNING_MSG(s);

#if KBE_PLATFORM == PLATFORM_WIN32
				if(count <= 25)
					DebugHelper::getSingleton().set_warningcolor();
				else
					DebugHelper::getSingleton().set_errorcolor();

				printf("[WARNING]: %s", s.c_str());
				DebugHelper::getSingleton().set_normalcolor();
#endif
			}

			Network::BundleBroadcast bhandler(*pNetworkInterface(), nport);
			if(!bhandler.good())
			{
				return false;
			}

			bhandler.itry(0);
			if(bhandler.pCurrPacket() != NULL)
			{
				bhandler.pCurrPacket()->resetPacket();
			}

			bhandler.newMessage(MachineInterface::onFindInterfaceAddr);
			MachineInterface::onFindInterfaceAddrArgs7::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
				componentType_, componentID_, findComponentType, pNetworkInterface()->intaddr().ip, bhandler.epListen().addr().port);

			if(!bhandler.broadcast())
			{
				ERROR_MSG("Components::findComponents: broadcast error!\n");
				return false;
			}
		
			int32 timeout = 1500000;
			bool showerr = true;
			MachineInterface::onBroadcastInterfaceArgs24 args;

RESTART_RECV:

			if(bhandler.receive(&args, 0, timeout, showerr))
			{
				bool isContinue = false;
				showerr = false;
				timeout = 1000000;

				do
				{
					if(isContinue)
					{
						try
						{
							args.createFromStream(*bhandler.pCurrPacket());
						}catch(MemoryStreamException &)
						{
							break;
						}
					}
					
					if(args.componentIDEx != componentID_)
					{
						WARNING_MSG(fmt::format("Components::findComponents: msg.componentID {} != {}.\n", 
							args.componentIDEx, componentID_));
						
						args.componentIDEx = 0;
						goto RESTART_RECV;
					}

					// ����Ҳ���
					if(args.componentType == UNKNOWN_COMPONENT_TYPE)
					{
						isContinue = true;
						continue;
					}

					INFO_MSG(fmt::format("Components::findComponents: found {}, addr:{}:{}\n",
						COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType),
						inet_ntoa((struct in_addr&)args.intaddr),
						ntohs(args.intport)));

					Components::getSingleton().addComponent(args.uid, args.username.c_str(), 
						(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.globalorderid, args.grouporderid, 
						args.intaddr, args.intport, args.extaddr, args.extport, args.extaddrEx, args.pid, args.cpu, args.mem, 
						args.usedmem, args.extradata, args.extradata1, args.extradata2, args.extradata3);

					isContinue = true;
				}while(bhandler.pCurrPacket()->length() > 0);

				// ��ֹ���յ������ݲ�����Ҫ������
				if(findComponentType == args.componentType)
				{
					// �������������� ��logger������������ȥ�� �������Ծ���ͬ����־
					if(findComponentType == (int8)LOGGER_TYPE)
					{
						findComponentTypes_[findIdx_] = -1;
						if(connectComponent(static_cast<COMPONENT_TYPE>(findComponentType), getUserUID(), 0) != 0)
						{
							ERROR_MSG(fmt::format("Components::findComponents: register self to {} is error!\n",
							COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));
							findIdx_++;
							//dispatcher().breakProcessing();
							return false;
						}
						else
						{
							findIdx_++;
							continue;
						}
					}
				}
				
				goto RESTART_RECV;
			}
			else
			{
				if(Components::getSingleton().getComponents((COMPONENT_TYPE)findComponentType).size() > 0)
				{
					findIdx_++;
					count = 0;
				}
				else
				{
					if(showerr)
					{
						ERROR_MSG("Components::findComponents: receive error!\n");
					}

					// �������Щ�������û�ҵ�������
					int helperComponentIdx = 0;

					while(1)
					{
						COMPONENT_TYPE helperComponentType = ALL_HELPER_COMPONENT_TYPE[helperComponentIdx++];
						if(helperComponentType == UNKNOWN_COMPONENT_TYPE)
						{
							break;
						}
						else if(findComponentType == helperComponentType)
						{
							WARNING_MSG(fmt::format("Components::findComponents: not found {}!\n",
								COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));

							findComponentTypes_[findIdx_] = -1; // ������־
							count = 0;
							findIdx_++;
							return false;
						}
					}
				}

				return false;
			}
		}

		state_ = 2;
		findIdx_ = 0;
		return false;
	}

	if(state_ == 2)
	{
		// ��ʼע�ᵽ���е����
		while(findComponentTypes_[findIdx_] != UNKNOWN_COMPONENT_TYPE)
		{
			if(dispatcher().hasBreakProcessing())
				return false;

			int8 findComponentType = findComponentTypes_[findIdx_];
			
			if(findComponentType == -1)
			{
				findIdx_++;
				return false;
			}

			INFO_MSG(fmt::format("Components::findComponents: register self to {}...\n",
				COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));

			if(connectComponent(static_cast<COMPONENT_TYPE>(findComponentType), getUserUID(), 0) != 0)
			{
				ERROR_MSG(fmt::format("Components::findComponents: register self to {} is error!\n",
				COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));
				//dispatcher().breakProcessing();
				return false;
			}

			findIdx_++;
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Components::onFoundAllComponents()
{
	INFO_MSG("Components::process(): Found all the components!\n");

#if KBE_PLATFORM == PLATFORM_WIN32
		DebugHelper::getSingleton().set_normalcolor();
		printf("[INFO]: Found all the components!\n");
		DebugHelper::getSingleton().set_normalcolor();
#endif
}

//-------------------------------------------------------------------------------------
bool Components::process()
{
	if(componentType_ == MACHINE_TYPE)
	{
		onFoundAllComponents();
		return false;
	}

	if(state_ == 0)
	{
		uint64 cidex = 0;

		DEBUG_MSG("Components::process(): Request for the process of identity...\n");

		while(cidex++ < 2)
		{
			if(dispatcher().hasBreakProcessing() || dispatcher().waitingBreakProcessing())
				return false;

			srand(KBEngine::getSystemTime());
			uint16 nport = KBE_PORT_START + (rand() % 1000);

			// ��������ڹ㲥UDP�����ύ�Լ������
			Network::BundleBroadcast bhandler(*pNetworkInterface(), nport);

			if(!bhandler.good())
			{
				continue;
			}

			bhandler.newMessage(MachineInterface::onBroadcastInterface);
			MachineInterface::onBroadcastInterfaceArgs24::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
				componentType_, componentID_, cidex, g_componentGlobalOrder, g_componentGroupOrder,
				pNetworkInterface()->intaddr().ip, pNetworkInterface()->intaddr().port,
				pNetworkInterface()->extaddr().ip, pNetworkInterface()->extaddr().port, g_kbeSrvConfig.getConfig().externalAddress, getProcessPID(),
				SystemInfo::getSingleton().getCPUPerByPID(), 0.f, (uint32)SystemInfo::getSingleton().getMemUsedByPID(), 0, 0, extraData1_, extraData2_, extraData3_, extraData4_, 
				pNetworkInterface()->intaddr().ip, bhandler.epListen().addr().port);
			
			bhandler.broadcast();

			// �ȴ�������Ϣ��������ڷ���˵������Ѿ���ʹ�ã��ý��̲��Ϸ���������������˳�
			// ���û�з���˵��û��machine�Դ˽�������������Գɹ�����
			int32 timeout = 500000;
			MachineInterface::onBroadcastInterfaceArgs24 args;

			if(bhandler.receive(&args, 0, timeout, false))
			{
				bool hasContinue = false;

				do
				{
					if(hasContinue)
					{
						try
						{
							args.createFromStream(*bhandler.pCurrPacket());
						}catch(MemoryStreamException &)
						{
							break;
						}
					}

					hasContinue = true;

					// �����δ֪���������һ��
					if(args.componentType == UNKNOWN_COMPONENT_TYPE)
						continue;

					if(args.componentID != componentID_)
						continue;

					ERROR_MSG(fmt::format("Components::process: found {}, addr:{}:{}\n",
						COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType),
						inet_ntoa((struct in_addr&)args.intaddr),
						ntohs(args.intport)));

					// ������ͬ��ݣ� ������˳���
					if(_pHandler)
						_pHandler->onIdentityillegal((COMPONENT_TYPE)args.componentType, args.componentID, args.pid, inet_ntoa((struct in_addr&)args.intaddr));

					return false;

				}while(bhandler.pCurrPacket()->length() > 0);
			}

			bhandler.close();
		}

		state_ = 1;

		return true;
	}
	else
	{
		static uint64 lastTime = timestamp();
			
		if(timestamp() - lastTime > uint64(stampsPerSecond()))
		{
			if(!findComponents())
			{
				if(state_ != 2)
					lastTime = timestamp();

				return true;
			}
		}
		else
			return true;
	}

	onFoundAllComponents();
	return false;
}

//-------------------------------------------------------------------------------------		
	
}
