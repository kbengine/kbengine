// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "fixed_messages.h"
#include "xml/xml.h"	
#include "resmgr/resmgr.h"	

namespace KBEngine { 

KBE_SINGLETON_INIT(Network::FixedMessages);

namespace Network
{

//-------------------------------------------------------------------------------------
FixedMessages::FixedMessages():
_infomap(),
_loaded(false)
{
	new Resmgr();
	Resmgr::getSingleton().initialize();
}

//-------------------------------------------------------------------------------------
FixedMessages::~FixedMessages()
{
	_infomap.clear();
}

//-------------------------------------------------------------------------------------
bool FixedMessages::loadConfig(std::string fileName, bool notFoundError)
{
	if(_loaded)
		return true;

	_loaded = true;

	TiXmlNode* node = NULL, *rootNode = NULL;

	SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes(fileName).c_str()));

	if(!xml->isGood())
	{
		if (notFoundError)
		{
#if KBE_PLATFORM == PLATFORM_WIN32
			printf("%s", (fmt::format("[ERROR]: FixedMessages::loadConfig: load {} is failed!\n", fileName.c_str())).c_str());
#endif

			if (DebugHelper::isInit())
			{
				ERROR_MSG(fmt::format("FixedMessages::loadConfig: load {} is failed!\n", fileName.c_str()));
			}
		}

		return false;
	}
	
	rootNode = xml->getRootNode();
	if(rootNode == NULL)
	{
		// root节点下没有子节点了
		return true;
	}

	XML_FOR_BEGIN(rootNode)
	{
		node = xml->enterNode(rootNode->FirstChild(), "id");

		FixedMessages::MSGInfo info;
		info.msgid = xml->getValInt(node);
		info.msgname = xml->getKey(rootNode);

		_infomap[info.msgname] = info;
	}
	XML_FOR_END(rootNode);

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
