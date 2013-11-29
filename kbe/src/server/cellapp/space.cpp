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

#include "cellapp.hpp"
#include "space.hpp"	
#include "entity.hpp"
#include "witness.hpp"	
#include "navigation/navmeshex.hpp"
#include "loadnavmesh_threadtasks.hpp"
#include "entitydef/entities.hpp"
#include "client_lib/client_interface.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
Space::Space(SPACE_ID spaceID):
id_(spaceID),
entities_(),
hasGeometry_(false),
loadGeometryPath_(),
pCell_(NULL),
rangeList_(),
pNavMeshHandle_(NULL)
{
}

//-------------------------------------------------------------------------------------
Space::~Space()
{
	SAFE_RELEASE(pCell_);
}

//-------------------------------------------------------------------------------------
PyObject* Space::__py_GetSpaceGeometryMapping(PyObject* self, PyObject* args)
{
	SPACE_ID spaceID = 0;

	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceGeometryMapping: (argssize != 1) is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(PyArg_ParseTuple(args, "I", &spaceID) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceGeometryMapping: args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	Space* space = Spaces::findSpace(spaceID);
	if(space == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceGeometryMapping: (spaceID=%u) not found!", 
			spaceID);

		PyErr_PrintEx(0);
		return false;
	}

	return PyUnicode_FromString(space->getGeometryPath().c_str());
}

//-------------------------------------------------------------------------------------
PyObject* Space::__py_AddSpaceGeometryMapping(PyObject* self, PyObject* args)
{
	SPACE_ID spaceID = 0;
	char* path = NULL;
	bool shouldLoadOnServer = true;
	PyObject* mapper = NULL;

	int argCount = PyTuple_Size(args);
	if(argCount < 3 || argCount > 4)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::addSpaceGeometryMapping: (argssize != 4) is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(argCount == 4)
	{
		if(PyArg_ParseTuple(args, "I|O|s|b", &spaceID, &mapper, &path, &shouldLoadOnServer) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::addSpaceGeometryMapping: args is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else
	{
		if(PyArg_ParseTuple(args, "I|O|s", &spaceID, &mapper, &path) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::addSpaceGeometryMapping: args is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}

	Space* space = Spaces::findSpace(spaceID);
	if(space == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::addSpaceGeometryMapping: (spaceID=%u respath=%s) not found!", 
			spaceID, path);

		PyErr_PrintEx(0);
		return false;
	}

	if(!space->addSpaceGeometryMapping(path, shouldLoadOnServer))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::addSpaceGeometryMapping: (spaceID=%u respath=%s) is error!", 
			spaceID, path);

		PyErr_PrintEx(0);
		return 0;
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
bool Space::addSpaceGeometryMapping(std::string respath, bool shouldLoadOnServer)
{
	INFO_MSG(boost::format("KBEngine::addSpaceGeometryMapping: spaceID=%1%, respath=%2%, shouldLoadOnServer=%3%!\n") %
		getID() % respath % shouldLoadOnServer);

	hasGeometry_ = true;
	if(loadGeometryPath_ == respath)
	{
		WARNING_MSG(boost::format("KBEngine::addSpaceGeometryMapping: spaceID=%1%, respath=%2% is exist!\n") %
			getID() % respath);

		return true;
	}

	loadGeometryPath_ = respath.c_str();

	if(shouldLoadOnServer)
		loadSpaceGeometry();

	SPACE_ENTITIES::const_iterator iter = this->entities().begin();
	for(; iter != this->entities().end(); iter++)
	{
		const Entity* pEntity = (*iter).get();

		if(pEntity || pEntity->isDestroyed() || !pEntity->hasWitness())
			continue;
		
		_addSpaceGeometryMappingToEntityClient(pEntity);
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Space::_addSpaceGeometryMappingToEntityClient(const Entity* pEntity)
{
	if(!hasGeometry_)
	{
		ERROR_MSG(boost::format("KBEngine::addSpaceGeometryMapping: spaceID=%1%, respath=%2%, hasGeometry = false!\n") %
			getID() % loadGeometryPath_);

		return;
	}

	if(!pEntity)
	{
		return;
	}

	if(pEntity->isDestroyed())
	{
		return;
	}

	if(!pEntity->hasWitness())
	{
		WARNING_MSG(boost::format("Space::_addSpaceGeometryMappingToEntityClient: entity %1% no client!\n") % 
			pEntity->getID());

		return;
	}

	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
	pForwardBundle->newMessage(ClientInterface::addSpaceGeometryMapping);
	(*pForwardBundle) << this->getID();
	(*pForwardBundle) << loadGeometryPath_;

	Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity->getID(), (*pSendBundle), (*pForwardBundle));
	pEntity->pWitness()->sendToClient(ClientInterface::addSpaceGeometryMapping, pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
}

//-------------------------------------------------------------------------------------
void Space::loadSpaceGeometry()
{
	KBE_ASSERT(pNavMeshHandle_ == NULL);
	Cellapp::getSingleton().threadPool().addTask(new LoadNavmeshTask(loadGeometryPath_, this->getID()));
}

//-------------------------------------------------------------------------------------
void Space::unLoadSpaceGeometry()
{
}

//-------------------------------------------------------------------------------------
void Space::onLoadedSpaceGeometryMapping(NavMeshHandle* pNavMeshHandle)
{
	pNavMeshHandle_ = pNavMeshHandle;
	INFO_MSG(boost::format("KBEngine::onLoadedSpaceGeometryMapping: spaceID=%1%, respath=%2%!\n") %
			getID() % loadGeometryPath_);
}

//-------------------------------------------------------------------------------------
void Space::onAllSpaceGeometryLoaded()
{
}

//-------------------------------------------------------------------------------------
void Space::update()
{
	if(pNavMeshHandle_ == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void Space::addEntityAndEnterWorld(Entity* pEntity, bool isRestore)
{
	addEntity(pEntity);
	addEntityToNode(pEntity);
	onEnterWorld(pEntity);
}

//-------------------------------------------------------------------------------------
void Space::addEntity(Entity* pEntity)
{
	pEntity->setSpaceID(this->id_);
	pEntity->spaceEntityIdx(entities_.size());
	entities_.push_back(pEntity);
}

//-------------------------------------------------------------------------------------
void Space::addEntityToNode(Entity* pEntity)
{
	pEntity->installRangeNodes(&rangeList_);
}

//-------------------------------------------------------------------------------------
void Space::removeEntity(Entity* pEntity)
{
	pEntity->setSpaceID(0);
	
	// 先获取到所在位置
	SPACE_ENTITIES::size_type idx = pEntity->spaceEntityIdx();

	KBE_ASSERT(idx < entities_.size());
	KBE_ASSERT(entities_[ idx ] == pEntity);

	// 如果有2个或以上的entity则将最后一个entity移至删除的这个entity所在位置
	Entity* pBack = entities_.back().get();
	pBack->spaceEntityIdx(idx);
	entities_[idx] = pBack;
	pEntity->spaceEntityIdx(SPACE_ENTITIES::size_type(-1));
	entities_.pop_back();

	onLeaveWorld(pEntity);

	// 这句必须在onLeaveWorld之后， 因为可能rangeTrigger需要参考pEntityRangeNode
	pEntity->uninstallRangeNodes(&rangeList_);

	// 如果没有entity了则需要销毁space, 因为space最少存在一个entity
	// 这个entity通常是spaceEntity
	if(entities_.empty())
	{
		Spaces::destroySpace(this->getID(), this->creatorID());
	}
}

//-------------------------------------------------------------------------------------
void Space::_onEnterWorld(Entity* pEntity)
{
	if(pEntity->hasWitness())
	{
		pEntity->pWitness()->onEnterSpace(this);
		_addSpaceGeometryMappingToEntityClient(pEntity);
	}
}

//-------------------------------------------------------------------------------------
void Space::onEnterWorld(Entity* pEntity)
{
	KBE_ASSERT(pEntity != NULL);
	
	// 如果是一个有Witness(通常是玩家)则需要将当前场景已经创建的有client部分的entity广播给他
	// 否则是一个普通的entity进入世界， 那么需要将这个entity广播给所有看见他的有Witness的entity。
	if(pEntity->hasWitness())
	{
		_onEnterWorld(pEntity);
	}
}

//-------------------------------------------------------------------------------------
void Space::onEntityAttachWitness(Entity* pEntity)
{
	KBE_ASSERT(pEntity != NULL && pEntity->hasWitness());
	_onEnterWorld(pEntity);
}

//-------------------------------------------------------------------------------------
void Space::onLeaveWorld(Entity* pEntity)
{
	if(!pEntity->getScriptModule()->hasClient())
		return;
	
	// 向其他人客户端广播自己的离开
	// 向客户端发送onLeaveWorld消息
	if(pEntity->hasWitness())
	{
		pEntity->pWitness()->onLeaveSpace(this);
	}
}

//-------------------------------------------------------------------------------------
bool Space::destroy(ENTITY_ID entityID)
{
	if(this->entities().size() == 0)
		return true;

	SPACE_ENTITIES entitieslog;
	
	Entity* creator = NULL;

	SPACE_ENTITIES::const_iterator iter = this->entities().begin();
	for(; iter != this->entities().end(); iter++)
	{
		const Entity* entity = (*iter).get();

		if(entity->getID() == this->creatorID())
			creator = const_cast<Entity*>(entity);
		else
			entitieslog.push_back((*iter));
	}

	iter = entitieslog.begin();
	for(; iter != entitieslog.end(); iter++)
	{
		Entity* entity = const_cast<Entity*>((*iter).get());
		entity->destroyEntity();
	}

	// 最后销毁创建者
	if(creator)
		creator->destroyEntity();

	return true;
}

//-------------------------------------------------------------------------------------
}
