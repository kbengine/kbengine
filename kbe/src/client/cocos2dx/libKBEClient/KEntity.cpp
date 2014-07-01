#include "KEntity.h"
#include "KBEApplication.h"

//void KBEngineClient::Entity::onRemoteMethodCall( std::string methodname,MemoryStream& s )
//{
//	//Todo decode stream to Entity ? method name first.
//
//}

bool KBEngineClient::Entity::isPlayer()
{
	return id == (int32) KBEngineClient::ClientApp::getInstance().entityID();
}
