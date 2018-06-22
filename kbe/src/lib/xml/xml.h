// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
				XML* xml = new XML("KBEngine.xml");
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
				XML* xml = new XML("KBEngine.xml");
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
			printf("%s", (fmt::format("TiXmlNode::openXML: {}, error!\n", pathbuf)).c_str());
#endif
			if(DebugHelper::isInit())
			{
				ERROR_MSG(fmt::format("TiXmlNode::openXML: {}, error!\n", pathbuf));
			}

			isGood_ = false;
			return false;
		}

		rootElement_ = txdoc_->RootElement();
		isGood_ = true;
		return true;
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

	bool getBool(const TiXmlNode* node)
	{
		std::string s = strutil::toUpper(getValStr(node));

		if (s == "TRUE")
		{
			return true;
		}
		else if (s == "FALSE")
		{
			return false;
		}

		return getValInt(node) > 0;
	}

protected:
	TiXmlDocument* txdoc_;
	TiXmlElement* rootElement_;
	bool isGood_;

};

}
 
#endif // KBE_XMLP_H
