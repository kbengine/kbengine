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

#include "navigation.hpp"
#include "resmgr/resmgr.hpp"
#include "thread/threadguard.hpp"

namespace KBEngine{

KBE_SINGLETON_INIT(Navigation);

//-------------------------------------------------------------------------------------
Navigation::Navigation():
navhandles_(),
mutex_()
{
}

//-------------------------------------------------------------------------------------
Navigation::~Navigation()
{
	KBEngine::thread::ThreadGuard tg(&mutex_); 
	KBEUnordered_map<std::string, NavigationHandle*>::iterator iter = navhandles_.begin();
	for(; iter != navhandles_.end(); iter++)
	{
		delete iter->second;
	}

	navhandles_.clear();
}

//-------------------------------------------------------------------------------------
bool Navigation::removeNavigation(std::string name)
{
	KBEngine::thread::ThreadGuard tg(&mutex_); 
	KBEUnordered_map<std::string, NavigationHandle*>::iterator iter = navhandles_.find(name);
	if(navhandles_.find(name) != navhandles_.end())
	{
		NavMeshHandle* pNavMeshHandle = (NavMeshHandle*)iter->second;
		dtFreeNavMeshQuery(pNavMeshHandle->navmeshQuery);
		dtFreeNavMesh(pNavMeshHandle->navmesh);
		navhandles_.erase(iter);
		delete pNavMeshHandle;

		DEBUG_MSG(boost::format("Navigation::removeNavigation: (%1%) is destroyed!\n") % name);
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
NavigationHandle* Navigation::findNavigation(std::string name)
{
	KBEngine::thread::ThreadGuard tg(&mutex_); 
	KBEUnordered_map<std::string, NavigationHandle*>::iterator iter = navhandles_.find(name);
	if(navhandles_.find(name) != navhandles_.end())
	{
		return iter->second;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
bool Navigation::hasNavigation(std::string name)
{
	KBEngine::thread::ThreadGuard tg(&mutex_); 
	return navhandles_.find(name) != navhandles_.end();
}

//-------------------------------------------------------------------------------------
NavigationHandle* Navigation::loadNavigation(std::string name)
{
	KBEngine::thread::ThreadGuard tg(&mutex_); 
	return loadNavmesh(name);
}

//-------------------------------------------------------------------------------------
NavMeshHandle* Navigation::loadNavmesh(std::string name)
{
	if(name == "")
		return NULL;

	std::string path = Resmgr::getSingleton().matchRes("spaces/" + name + "/" + name + ".navmesh");

	if(navhandles_.find(name) != navhandles_.end())
	{
		return NULL;
	}

	FILE* fp = fopen(path.c_str(), "rb");
	if (!fp)
	{
		ERROR_MSG(boost::format("Navigation::loadNavmesh: open(%1%) is error!\n") % path);
		return NULL;
	}

	bool safeStorage = true;
    int pos = 0;
    int size = sizeof(NavMeshSetHeader);
	
	fseek(fp, 0, SEEK_END); 
	size_t flen = ftell(fp); 
	fseek(fp, 0, SEEK_SET); 

	uint8* data = new uint8[flen];
	if(data == NULL)
	{
		ERROR_MSG(boost::format("Navigation::loadNavmesh: open(%1%), memory(size=%2%) error!\n") % path % flen);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

	size_t readsize = fread(data, 1, flen, fp);
	if(readsize != flen)
	{
		ERROR_MSG(boost::format("Navigation::loadNavmesh: open(%1%), read(size=%2% != %3%) error!\n") % path % readsize % flen);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

    if (readsize < sizeof(NavMeshSetHeader))
	{
		ERROR_MSG(boost::format("Navigation::loadNavmesh: open(%1%), NavMeshSetHeader is error!\n") % path);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

    NavMeshSetHeader header;
	memcpy(&header, data, size);

    pos += size;

	if (header.version != NavMeshHandle::RCN_NAVMESH_VERSION)
    {
		ERROR_MSG(boost::format("Navigation::loadNavmesh: version(%1%) is not match(%2%)!\n") % header.version % NavMeshHandle::RCN_NAVMESH_VERSION);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
        return NULL;
    }

    dtNavMesh* mesh = dtAllocNavMesh();
    if (!mesh)
    {
		ERROR_MSG("Navigation::loadNavmesh: dtAllocNavMesh is failed!\n");
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
        return NULL;
    }

    dtStatus status = mesh->init(&header.params);
    if (dtStatusFailed(status))
    {
		ERROR_MSG(boost::format("Navigation::loadNavmesh: mesh init is error(%1%)!\n") % status);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
	    return NULL;
    }

    // Read tiles.
    bool success = true;
    for (int i = 0; i < header.tileCount; ++i)
    {
	    NavMeshTileHeader tileHeader;
        size = sizeof(NavMeshTileHeader);
	    memcpy(&tileHeader, &data[pos], size);
        pos += size;

        size = tileHeader.dataSize;
		if (!tileHeader.tileRef || !tileHeader.dataSize)
        {
            success = false;
            status = DT_FAILURE + DT_INVALID_PARAM;
		    break;
        }
		
	    unsigned char* tileData = 
            (unsigned char*)dtAlloc(size, DT_ALLOC_PERM);
	    if (!tileData)
        {
            success = false;
            status = DT_FAILURE + DT_OUT_OF_MEMORY;
            break;
        }
        memcpy(tileData, &data[pos], size);
        pos += size;

	    status = mesh->addTile(tileData
            , size
			, (safeStorage ? DT_TILE_FREE_DATA : 0)
            , tileHeader.tileRef
            , 0);

        if (dtStatusFailed(status))
        {
            success = false;
            break;
        }
    }

	fclose(fp);
	SAFE_RELEASE_ARRAY(data);

    if (!success)
    {
		ERROR_MSG(boost::format("Navigation::loadNavmesh:  error(%1%)!\n") % status);
        dtFreeNavMesh(mesh);
		return NULL;
    }

	NavMeshHandle* pNavMeshHandle = new NavMeshHandle();
	pNavMeshHandle->navmesh = mesh;
	pNavMeshHandle->navmeshQuery = new dtNavMeshQuery();
	pNavMeshHandle->navmeshQuery->init(mesh, 1024);
	pNavMeshHandle->name = name;
	navhandles_[name] = pNavMeshHandle;

    uint32 tileCount = 0;
    uint32 nodeCount = 0;
    uint32 polyCount = 0;
    uint32 vertCount = 0;
    uint32 triCount = 0;
    uint32 triVertCount = 0;
    uint32 dataSize = 0;

	const dtNavMesh* navmesh = mesh;
    for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        tileCount ++;
        nodeCount += tile->header->bvNodeCount;
        polyCount += tile->header->polyCount;
        vertCount += tile->header->vertCount;
        triCount += tile->header->detailTriCount;
        triVertCount += tile->header->detailVertCount;
        dataSize += tile->dataSize;

		// DEBUG_MSG(boost::format("Navigation::loadNavmesh: verts(%1%, %2%, %3%)\n") % tile->verts[0] % tile->verts[1] % tile->verts[2]);
    }

	DEBUG_MSG(boost::format("Navigation::loadNavmesh: (%1%)\n") % name);
	DEBUG_MSG(boost::format("\t==> tiles loaded: %1%\n") % tileCount);
	DEBUG_MSG(boost::format("\t==> BVTree nodes: %1%\n") % nodeCount);
    DEBUG_MSG(boost::format("\t==> %1% polygons (%2% vertices)\n") % polyCount % vertCount);
    DEBUG_MSG(boost::format("\t==> %1% triangles (%2% vertices)\n") % triCount % triVertCount);
    DEBUG_MSG(boost::format("\t==> %.2f MB of data (not including pointers)\n") % (((float)dataSize / sizeof(unsigned char)) / 1048576));
	return pNavMeshHandle;
}

//-------------------------------------------------------------------------------------		
}
