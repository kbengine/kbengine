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

/*
	xml ��д��
		����:	
				<root>
					<server>
						<ip>172.16.0.12</ip>
						<port>6000</port>
					</server>
				</root>
		    	--------------------------------------------------------------------------------
				XML* xml = new XML("KBEngine.xml");
				TiXmlNode* node = xml->getRootNode("server");

				XML_FOR_BEGIN(node)
				{
					printf("%s--%s\n", xml->getKey(node).c_str(), xml->getValStr(node->FirstChild()).c_str());
				}
				XML_FOR_END(node);
				
				delete xml;
		���:
				---ip---172.16.0.12
				---port---6000
				

		����2:
				XML* xml = new XML("KBEngine.xml");
				TiXmlNode* serverNode = xml->getRootNode("server");
				
				TiXmlNode* node;
				node = xml->enterNode(serverNode, "ip");	
				printf("%s\n", xml->getValStr(node).c_str() );	

				node = xml->enterNode(serverNode, "port");		
				printf("%s\n", xml->getValStr(node).c_str() );	
			
		���:
			172.16.0.12
			6000
*/

#ifndef KBE_XMLP_H
#define KBE_XMLP_H

#include <string>
#include "helper/debug_helper.h"
#include "common/common.h"
#include "common/smartpointer.h"
#include "dependencies/tinyxml/tinyxml.h"

namespace KBEngine{

#define XML_FOR_BEGIN(node)																\
		do																				\
		{																				\
		if(node->Type() != TiXmlNode::TINYXML_ELEMENT)									\
				continue;																\
			
#define XML_FOR_END(node)																\
	}while((node = node->NextSibling()));												\
			
class  XML : public RefCountable
{
public:
	XML(void):
		txdoc_(NULL),
		rootElement_(NULL),
		isGood_(false)
	{
	}

	XML(const char* xmlFile):
		txdoc_(NULL),
		rootElement_(NULL),
		isGood_(false)
	{
		openSection(xmlFile);
	}
	
	~XML(void){
		if(txdoc_){
			txdoc_->Clear();
			delete txdoc_;
			txdoc_ = NULL;
			rootElement_ = NULL;
		}
	}

	bool isGood() const{ return isGood_; }

	bool openSection(const char* xmlFile)
	{
		char pathbuf[MAX_PATH];
		kbe_snprintf(pathbuf, MAX_PATH, "%s", xmlFile);

		txdoc_ = new TiXmlDocument((char*)&pathbuf);

		if(!txdoc_->LoadFile())
		{
#if KBE_PLATFORM == PLATFORM_WIN32
			printf("%s", (fmt::format("TiXmlNode::openXML: {}, is error!\n", pathbuf)).c_str());
#endif
			if(DebugHelper::isInit())
			{
				ERROR_MSG(fmt::format("TiXmlNode::openXML: {}, is error!\n", pathbuf));
			}

			isGood_ = false;
			return false;
		}

		rootElement_ = txdoc_->RootElement();
		isGood_ = true;
		return true;
	}

	/**��ȡ��Ԫ��*/
	TiXmlElement* getRootElement(void){return rootElement_;}

	/**��ȡ���ڵ㣬 ������keyΪ��Χ���ڵ��µ�ĳ���ӽڵ��*/
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

	/**ֱ�ӷ���Ҫ�����key�ڵ�ָ��*/
	TiXmlNode* enterNode(TiXmlNode* node, const char* key)
	{
		do
		{
			if(node->Type() != TiXmlNode::TINYXML_ELEMENT)
				continue;

			if (getKey(node) == key)
			{
				TiXmlNode* childNode = node->FirstChild();
				do
				{
					if (!childNode || childNode->Type() != TiXmlNode::TINYXML_COMMENT)
						break;
				}
				while ((childNode = childNode->NextSibling()));

				return childNode;
			}

		}
		while((node = node->NextSibling()));

		return NULL;
	}

	/**�Ƿ��������һ��key*/
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
	
	TiXmlDocument* getTxdoc() const { return txdoc_; }

	std::string getKey(const TiXmlNode* node)
	{
		if(node == NULL)
			return "";

		return strutil::kbe_trim(node->Value());
	}

	std::string getValStr(const TiXmlNode* node)
	{
		const TiXmlText* ptext = node->ToText();
		if(ptext == NULL)
			return "";

		return strutil::kbe_trim(ptext->Value());
	}

	std::string getVal(const TiXmlNode* node)
	{
		const TiXmlText* ptext = node->ToText();
		if(ptext == NULL)
			return "";

		return ptext->Value();
	}

	int getValInt(const TiXmlNode* node)
	{
		const TiXmlText* ptext = node->ToText();
		if(ptext == NULL)
			return 0;

		return atoi(strutil::kbe_trim(ptext->Value()).c_str());
	}

	double getValFloat(const TiXmlNode* node)
	{
		const TiXmlText* ptext = node->ToText();
		if(ptext == NULL)
			return 0.f;

		return atof(strutil::kbe_trim(ptext->Value()).c_str());
	}

protected:
	TiXmlDocument* txdoc_;
	TiXmlElement* rootElement_;
	bool isGood_;
};

}
 
#endif // KBE_XMLP_H
