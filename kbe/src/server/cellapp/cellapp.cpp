#include "cellapp.hpp"
#include "entity.hpp"
#include "entities.hpp"
#include "cellapp_interface.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(CellApp);

//-------------------------------------------------------------------------------------
CellApp::CellApp(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
  EntityApp(dispatcher, ninterface, componentType),
    idClient_(NULL),
    entities_(NULL),
    gameTimer_()
{
	// KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &CellAppInterface::messageHandlers;	
}

//-------------------------------------------------------------------------------------
CellApp::~CellApp()
{
}

//-------------------------------------------------------------------------------------
bool CellApp::installPyModules()
{
	Entities::installScript(NULL);
	Entity::installScript(getScript().getModule());

	registerScript(Entity::getScriptType());
	
	entities_ = new Entities();
	registerPyObjectToScript("entities", entities_);
	return true;
}

//-------------------------------------------------------------------------------------
bool CellApp::uninstallPyModules()
{	
	S_RELEASE(entities_);
	Entities::uninstallScript();
	Entity::uninstallScript();
	return true;
}

//-------------------------------------------------------------------------------------
bool CellApp::run()
{
	idClient_->onAddRange(1, 500);
	Entity* e = createEntity("Avatar", NULL);
	registerPyObjectToScript("avatar", e);
	PyRun_SimpleString("print ('888888888888888888888', KBEngine.avatar.id)");
	DEBUG_MSG("kbe:python is init successfully!!! %d\n", 88);
	SmartPointer<PyObject> testsmartpointer(::PyBytes_FromString("test"));
	testsmartpointer.clear();

	CRITICAL_MSG("hahahah %d\n", 1111);
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void CellApp::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
			this->handleGameTick();
			break;
		case TIMEOUT_LOADING_TICK:
		{
			break;
		}
	}
}

//-------------------------------------------------------------------------------------
void CellApp::handleGameTick()
{
	// time_t t = ::time(NULL);
	// DEBUG_MSG("CellApp::handleGameTick[%"PRTime"]:%d\n", t, time_);
	
	time_++;
	
	getNetworkInterface().handleChannels(&CellAppInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool CellApp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool CellApp::initializeEnd()
{
	idClient_ = new IDClient<ENTITY_ID>;
	
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	
	g_kbeSrvConfig.printCellapp();
	return true;
}

//-------------------------------------------------------------------------------------
void CellApp::finalise()
{
	SAFE_RELEASE(idClient_);
	gameTimer_.cancel();
}

//-------------------------------------------------------------------------------------
Entity* CellApp::createEntity(const char* entityType, PyObject* params, bool isInitializeScript, ENTITY_ID eid)
{
	// 检查ID是否足够, 不足返回NULL
	if(eid <= 0 && idClient_->getSize() == 0)
	{
		PyErr_SetString(PyExc_SystemError, "App::createEntity: is Failed. not enough entityIDs.");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ScriptModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL || !sm->hasCell())
	{
		PyErr_Format(PyExc_TypeError, "App::createEntity: entityType [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* obj = sm->createObject();

	// 判断是否要分配一个新的id
	ENTITY_ID id = eid;
	if(id <= 0)
		id = idClient_->alloc();

	// 执行Entity的构造函数
	Entity* entity = new(obj) Entity(id, sm);

	// 创建名字空间
	entity->createNamespace(params);

	// 将entity加入entities
	entities_->add(id, entity); 
	
	// 检查ID的足够性，不足则申请
	//checkEntityIDEnough();

	// 初始化脚本
	if(isInitializeScript)
		entity->initializeScript();
	
	INFO_MSG("App::createEntity: new %s (%ld).\n", entityType, id);
	return entity;
}

//-------------------------------------------------------------------------------------
Entity* CellApp::findEntity(ENTITY_ID eid)
{
	return entities_->find(eid);
}

//-------------------------------------------------------------------------------------
bool CellApp::destroyEntity(ENTITY_ID entityID)
{
	return entities_->destroy(entityID);
}

//-------------------------------------------------------------------------------------

}
