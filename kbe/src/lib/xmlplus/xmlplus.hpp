/* 
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
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
#include <errno.h>
#endif

#define LIB_DLLAPI  __declspec(dllexport)

#ifdef __cplusplus  
extern "C" {  
#endif  

#ifdef __cplusplus  
}
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
protected:
	TiXmlDocument* m_txdoc_;
	TiXmlElement* m_rootElement_;
public:
	XmlPlus(void):m_txdoc_(NULL),m_rootElement_(NULL)
	{
	}
	XmlPlus(const char* xmlFile){
		openSection(xmlFile);
	}
	
	~XmlPlus(void){
		if(m_txdoc_){
			m_txdoc_->Clear();
			delete m_txdoc_;
			m_txdoc_ = NULL;
			m_rootElement_ = NULL;
		}
	}

	TiXmlNode* openSection(const char* xmlFile)
	{
		char pathbuf[255];
		sprintf(pathbuf, "%s", xmlFile);

		m_txdoc_ = new TiXmlDocument((char*)&pathbuf);
		if(!m_txdoc_->LoadFile()){
			ERROR_MSG("load xml from %s is error!\n", pathbuf);
			return NULL;
		}

		m_rootElement_ = m_txdoc_->RootElement();
		return getRootNode();
	}

	/**获取根元素*/
	TiXmlElement* getRootElement(void){return m_rootElement_;}

	/**获取根节点， 带参数key为范围根节点下的某个子节点根*/
	TiXmlNode* getRootNode(const char* key = "")
	{
		if(strlen(key) > 0)
			return m_rootElement_->FirstChild(key)->FirstChild();
		return m_rootElement_->FirstChild();
	}

	/**直接返回要进入的key节点指针*/
	TiXmlNode* enterNode(TiXmlNode* node, const char* key)
	{
		do{
			if(getKey(node) == key)
				return node->FirstChild();
		}while((node = node->NextSibling()));

		return NULL;
	}

	/**是否存在这样一个key*/
	bool hasNode(TiXmlNode* node, const char* key)
	{
		do{
			if(getKey(node) == key)
				return true;
		}while((node = node->NextSibling()));

		return false;	
	}

	std::string getKey(const TiXmlNode* node){return node->Value();}
	std::string getValStr(const TiXmlNode* node){return node->ToText()->Value();}
	int getValInt(const TiXmlNode* node){return atoi(node->ToText()->Value());}
	double getValFloat(const TiXmlNode* node){return atof(node->ToText()->Value());}
};

}
 
#endif