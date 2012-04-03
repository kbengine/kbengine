#include "xmlplus.hpp"
namespace KBEngine{
 
//-------------------------------------------------------------------------------------
XmlPlus::XmlPlus()
{
	m_txdoc_ = NULL;
	m_rootElement_ = NULL;
}

//-------------------------------------------------------------------------------------
XmlPlus::XmlPlus(const char* xmlFile)
{
	openSection(xmlFile);
}

//-------------------------------------------------------------------------------------
XmlPlus::~XmlPlus()
{
	if(m_txdoc_){
		m_txdoc_->Clear();
		delete m_txdoc_;
		m_txdoc_ = NULL;
		m_rootElement_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
TiXmlNode * XmlPlus::openSection(const char* xmlFile)
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

//-------------------------------------------------------------------------------------
TiXmlElement* XmlPlus::getRootElement(void){
	return m_rootElement_;
}

//-------------------------------------------------------------------------------------
TiXmlNode* XmlPlus::getRootNode(const char* key)
{
	if(strlen(key) > 0)
		return m_rootElement_->FirstChild(key)->FirstChild();
	return m_rootElement_->FirstChild();
}

//-------------------------------------------------------------------------------------
TiXmlNode* XmlPlus::enterNode(TiXmlNode* node, const char* key)
{
	do{
		if(getKey(node) == key)
			return node->FirstChild();
	}while((node = node->NextSibling()));

	return NULL;
}

bool XmlPlus::hasNode(TiXmlNode* node, const char* key)
{
	do{
		if(getKey(node) == key)
			return true;
	}while((node = node->NextSibling()));

	return false;	
}

//-------------------------------------------------------------------------------------
std::string XmlPlus::getValStr(const TiXmlNode* node)
{
	return node->ToText()->Value();
}

//-------------------------------------------------------------------------------------
int XmlPlus::getValInt(const TiXmlNode* node)
{
	return atoi(node->ToText()->Value());
}

//-------------------------------------------------------------------------------------
double XmlPlus::getValFloat(const TiXmlNode* node)
{
	return atof(node->ToText()->Value());
}

//-------------------------------------------------------------------------------------
std::string XmlPlus::getKey(const TiXmlNode* node)
{
	return node->Value();
}

}