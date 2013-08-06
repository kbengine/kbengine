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
#include "globaldata_client.hpp"
#include "components.hpp"
#include "serverapp.hpp"
#include "network/channel.hpp"

#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{ 


SCRIPT_METHOD_DECLARE_BEGIN(GlobalDataClient)
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(GlobalDataClient)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(GlobalDataClient)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(GlobalDataClient, 0, 0, &Map::mappingMethods, 0, 0)
	
//-------------------------------------------------------------------------------------
GlobalDataClient::GlobalDataClient(COMPONENT_TYPE componentType, GlobalDataServer::DATA_TYPE dataType):
script::Map(getScriptType(), false),
serverComponentType_(componentType),
dataType_(dataType)
{
}

//-------------------------------------------------------------------------------------
GlobalDataClient::~GlobalDataClient()
{
}

//-------------------------------------------------------------------------------------
bool GlobalDataClient::write(PyObject* pyKey, PyObject* pyValue)
{
	bool ret = false;
	if(pyKey && pyValue)
	{
		if (PyDict_SetItem(pyDict_, pyKey, pyValue) == -1)
		{
			ERROR_MSG(boost::format("Map::write: is eror! key=%1%, val=%2%\n") % 
				PyBytes_AsString(pyKey) % PyBytes_AsString(pyValue));
		}
		else
		{
			ret = true;
		}
	}
	else
	{
		ERROR_MSG(boost::format("Map::write:unpickle is error. key=%1%, val=%2%\n") %
			PyBytes_AsString(pyKey) % PyBytes_AsString(pyValue));

		PyErr_Print();
	}

	// Py_XDECREF(pyKey);
	Py_XDECREF(pyValue);
	return ret;	
}

//-------------------------------------------------------------------------------------
bool GlobalDataClient::del(PyObject* pyKey)
{
	bool ret = false;

	if(pyKey)
	{
		if (PyDict_GetItem(pyDict_, pyKey) && PyDict_DelItem(pyDict_, pyKey) == -1)
		{
			ERROR_MSG(boost::format("Map::del: delete key is failed! key=%1%.\n") % PyBytes_AsString(pyKey));
			PyErr_Clear();
		}
		else
		{
			ret = true;
		}
		Py_DECREF(pyKey);
	}
	else
	{
		ERROR_MSG(boost::format("Map::del: delete key is error! key=%1%.\n") % PyBytes_AsString(pyKey));
		PyErr_Print();
	}

	return ret;	
}

//-------------------------------------------------------------------------------------
void GlobalDataClient::onDataChanged(std::string& key, std::string& value, bool isDelete)
{
	Components::COMPONENTS& channels = Components::getSingleton().getComponents(serverComponentType_);
	Components::COMPONENTS::iterator iter1 = channels.begin();
	uint8 dataType = dataType_;
	ArraySize slen = 0;

	for(; iter1 != channels.end(); iter1++)
	{
		Mercury::Channel* lpChannel = iter1->pChannel;
		KBE_ASSERT(lpChannel != NULL);
		
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		
		(*pBundle).newMessage(DbmgrInterface::onBroadcastGlobalDataChange);
		
		(*pBundle) << dataType;
		(*pBundle) << isDelete;

		slen = key.size();
		(*pBundle) << slen;
		(*pBundle).assign(key.data(), slen);

		if(!isDelete)
		{
			slen = value.size();
			(*pBundle) << slen;
			(*pBundle).assign(value.data(), slen);
		}

		(*pBundle) << g_componentType;

		(*pBundle).send(lpChannel->networkInterface(), lpChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
}
