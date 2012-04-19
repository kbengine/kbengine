#include "entity_app.hpp"
namespace KBEngine{

//-------------------------------------------------------------------------------------
EntityApp::EntityApp(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
ServerApp(dispatcher, ninterface, componentType)
{
}

//-------------------------------------------------------------------------------------
EntityApp::~EntityApp()
{
}

//-------------------------------------------------------------------------------------		
}
