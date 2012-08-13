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
#include "fixed_messages.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "resmgr/resmgr.hpp"	

namespace KBEngine { 

KBE_SINGLETON_INIT(Mercury::FixedMessages);

namespace Mercury
{

//-------------------------------------------------------------------------------------
FixedMessages::FixedMessages():
_infomap()
{
	Resmgr::initialize();
}

//-------------------------------------------------------------------------------------
FixedMessages::~FixedMessages()
{
	_infomap.clear();
}

//-------------------------------------------------------------------------------------
bool FixedMessages::loadConfig(std::string fileName)
{
	TiXmlNode* node = NULL, *rootNode = NULL;

	XmlPlus* xml = new XmlPlus(Resmgr::matchRes(fileName).c_str());

	if(!xml->isGood())
	{
		ERROR_MSG(" FixedMessages::loadConfig: load %s is failed!\n", fileName.c_str());
		SAFE_RELEASE(xml);
		return false;
	}
	
	rootNode = xml->getRootNode();
	XML_FOR_BEGIN(rootNode)
	{
		node = xml->enterNode(rootNode->FirstChild(), "id");

		FixedMessages::MSGInfo info;
		info.msgid = xml->getValInt(node);
		_infomap[xml->getKey(rootNode)] = info;
	}
	XML_FOR_END(rootNode);

	SAFE_RELEASE(xml);
	return true;
}

//-------------------------------------------------------------------------------------
FixedMessages::MSGInfo* FixedMessages::isFixed(const char* msgName)
{
	MSGINFO_MAP::iterator iter = _infomap.find(msgName);
	if(iter != _infomap.end())
	{
		MSGInfo* infos = &iter->second;
		return infos;
	}
	
	return NULL;
}

//-------------------------------------------------------------------------------------
bool FixedMessages::isFixed(MessageID msgid)
{
	MSGINFO_MAP::iterator iter = _infomap.begin();
	while (iter != _infomap.end())
	{
		FixedMessages::MSGInfo& infos = iter->second;
		if(infos.msgid == msgid)
			return true;

		++iter;
	}

	return false;
}

//-------------------------------------------------------------------------------------
}
}
