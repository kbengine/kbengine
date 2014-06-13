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

/*
	xml 读写：
		例子:	
				<root>
					<server>
						<ip>172.16.0.12</ip>
						<port>6000</port>
					</server>
				</root>
		    	--------------------------------------------------------------------------------
				XmlPlus* xml = new XmlPlus("KBEngine.xml");
				TiXmlNode* node = xml->getRootNode("server");

				XML_FOR_BEGIN(node)
				{
					printf("%s--%s\n", xml->getKey(node).c_str(), xml->getValStr(node->FirstChild()).c_str());
				}
				XML_FOR_END(node);
				
				delete xml;
		输出:
				---ip---172.16.0.12
				---port---6000
				

		例子2:
				XmlPlus* xml = new XmlPlus("KBEngine.xml");
				TiXmlNode* serverNode = xml->getRootNode("server");
				
				TiXmlNode* node;
				node = xml->enterNode(serverNode, "ip");	
				printf("%s\n", xml->getValStr(node).c_str() );	

				node = xml->enterNode(serverNode, "port");		
				printf("%s\n", xml->getValStr(node).c_str() );	
			
		输出:
			172.16.0.12
			6000
*/

#ifndef __XMLPLUS__
#define __XMLPLUS__

// common include	
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "third_party/tinyxml/tinyxml.h"

// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

#define XML_FOR_BEGIN(node)																\
		do																				\
		{																				\
		if(node->Type() != TiXmlNode::TINYXML_ELEMENT)									\
				continue;																\
			
#define XML_FOR_END(node)																\
	}while((node = node->NextSibling()));												\
			
class  XmlPlus : public RefCountable
{
public:
	XmlPlus(void):
		txdoc_(NULL),
		rootElement_(NULL),
		isGood_(false)
	{
	}

	XmlPlus(const char* xmlFile):
		txdoc_(NULL),
		rootElement_(NULL),
		isGood_(false)
	{
		isGood_ = openSection(xmlFile) != NULL || rootElement_ != NULL;
	}
	
	~XmlPlus(void){
		if(txdoc_){
			txdoc_->Clear();
			delete txdoc_;
			txdoc_ = NULL;
			rootElement_ = NULL;
		}
	}

	bool isGood()const{ return isGood_; }

	TiXmlNode* openSection(const char* xmlFile)
	{
		char pathbuf[255];
		kbe_snprintf(pathbuf, 255, "%s", xmlFile);

		txdoc_ = new TiXmlDocument((char*)&pathbuf);

		if(!txdoc_->LoadFile())
		{
#if KBE_PLATFORM == PLATFORM_WIN32
			printf("%s", (boost::format("TiXmlNode::openXML: %1%, is error!\n") % pathbuf).str().c_str());
#endif
			if(DebugHelper::isInit())
			{
//				ERROR_MSG(boost::format("TiXmlNode::openXML: %1%, is error!\n") % pathbuf);
			}

			return NULL;
		}

		rootElement_ = txdoc_->RootElement();
		return getRootNode();
	}

	/**获取根元素*/
	TiXmlElement* getRootElement(void){return rootElement_;}

	/**获取根节点， 带参数key为范围根节点下的某个子节点根*/
	TiXmlNode* getRootNode(const char* key = "")
	{
		if(rootElement_ == NULL)
			return rootElement_;

		if(strlen(key) > 0){
			TiXmlNode* node = rootElement_->FirstChild(key);
			if(node == NULL)
				return NULL;
			return node->FirstChild();
		}
		return rootElement_->FirstChild();
	}

	/**直接返回要进入的key节点指针*/
	TiXmlNode* enterNode(TiXmlNode* node, const char* key)
	{
		do{
			if(node->Type() != TiXmlNode::TINYXML_ELEMENT)
				continue;

			if(getKey(node) == key)
				return node->FirstChild();

		}while((node = node->NextSibling()));

		return NULL;
	}

	/**是否存在这样一个key*/
	bool hasNode(TiXmlNode* node, const char* key)
	{
		do{
			if(node->Type() != TiXmlNode::TINYXML_ELEMENT)
				continue;

			if(getKey(node) == key)
				return true;

		}while((node = node->NextSibling()));

		return false;	
	}
	
	TiXmlDocument* getTxdoc()const { return txdoc_; }

	std::string getKey(const TiXmlNode* node){return strutil::kbe_trim(node->Value());}
	std::string getValStr(const TiXmlNode* node){return strutil::kbe_trim(node->ToText()->Value());}
	std::string getVal(const TiXmlNode* node){return node->ToText()->Value();}
	int getValInt(const TiXmlNode* node){return atoi(strutil::kbe_trim(node->ToText()->Value()).c_str());}
	double getValFloat(const TiXmlNode* node){return atof(strutil::kbe_trim(node->ToText()->Value()).c_str());}
protected:
	TiXmlDocument* txdoc_;
	TiXmlElement* rootElement_;
	bool isGood_;
};

}
 
#endif
