#include "space.h"
#include "OgreApplication.h"

//-------------------------------------------------------------------------------------
Space::Space(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
			 OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr):
mRoot(pOgreRoot),
mActiveCamera(0),
mSceneMgr(0),
mWindow(pRenderWin),
mInputManager(pInputMgr),
mTrayMgr(pTrayMgr),
mCameraMan(0),
mRaySceneQuery(NULL),
mVertexBufferSize(0),
mIndexBufferSize(0),
mVertexBuffer(0),
mIndexBuffer(0)
{
}

//-------------------------------------------------------------------------------------
Space::~Space(void)
{
	if(mVertexBuffer) OGRE_FREE(mVertexBuffer, Ogre::MEMCATEGORY_GEOMETRY);
	if(mIndexBuffer) OGRE_FREE(mIndexBuffer, Ogre::MEMCATEGORY_GEOMETRY);
	if(mRaySceneQuery)mSceneMgr->destroyQuery(mRaySceneQuery);
	if (mCameraMan) delete mCameraMan;
	if(mWindow) mWindow->removeAllViewports();
	if(mRoot && mSceneMgr)mRoot->destroySceneManager(mSceneMgr);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		"/Space::~Space", "FileSystem", "Space");

	Ogre::ResourceGroupManager::getSingleton().clearResourceGroup("Space");

	OgreApplication::getSingleton().setCurrCameraMan(NULL);
}

//-------------------------------------------------------------------------------------
bool Space::setup()
{
	setupResources();

	// Create the scene
	createScene();

	return true;
}

//-------------------------------------------------------------------------------------
bool Space::pickEntity(Ogre::RaySceneQuery* mRaySceneQuery, Ogre::Ray &ray, Ogre::Entity **result, Ogre::uint32 querymask, Ogre::uint32 mVisibilityMask, Ogre::Vector3 &hitpoint, const Ogre::String& excludeobject, Ogre::Real max_distance)
{
    mRaySceneQuery->setRay(ray);
    mRaySceneQuery->setQueryMask(querymask);
    mRaySceneQuery->setSortByDistance(true);
    
    if (mRaySceneQuery->execute().size() <= 0) return (false);

    // at this point we have raycast to a series of different objects bounding boxes.
    // we need to test these different objects to see which is the first polygon hit.
    // there are some minor optimizations (distance based) that mean we wont have to
    // check all of the objects most of the time, but the worst case scenario is that
    // we need to test every triangle of every object.
    Ogre::Real closest_distance = max_distance;
    Ogre::Vector3 closest_result;
    Ogre::RaySceneQueryResult &query_result = mRaySceneQuery->getLastResults();

    for (size_t qr_idx = 0; qr_idx < query_result.size(); qr_idx++)
    {
        // stop checking if we have found a raycast hit that is closer
        // than all remaining entities
        if ((closest_distance >= 0.0f) && (closest_distance < query_result[qr_idx].distance))
            break;
    
        // only check this result if its a hit against an entity
        if ((query_result[qr_idx].movable != NULL) && (query_result[qr_idx].movable->getMovableType().compare("Entity") == 0))
        {
            // get the entity to check
            Ogre::Entity *pentity = static_cast<Ogre::Entity*>(query_result[qr_idx].movable);
			
			if(mVisibilityMask != 0)
			{
				if(!(pentity->getVisibilityFlags() & mVisibilityMask))
					continue;
			}

            if(!pentity->getVisible() || (pentity->getName() == excludeobject)) 
                continue;

            // mesh data to retrieve
            size_t vertex_count;
            size_t index_count;

            // get the mesh information
            getMeshData(pentity->getMesh(), vertex_count, index_count, 
                        pentity->getParentNode()->_getDerivedPosition(),
                        pentity->getParentNode()->_getDerivedOrientation(),
                        pentity->getParentNode()->_getDerivedScale());

            // test for hitting individual triangles on the mesh
            bool new_closest_found = false;
            for (int i = 0; i < static_cast<int>(index_count); i += 3)
            {
                assert(mIndexBuffer[i] < vertex_count);
                assert(mIndexBuffer[i + 1] < vertex_count);
                assert(mIndexBuffer[i + 2] < vertex_count);
                // check for a hit against this triangle
                std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, mVertexBuffer[mIndexBuffer[i]],
                         mVertexBuffer[mIndexBuffer[i+1]], mVertexBuffer[mIndexBuffer[i+2]], true, false);

                // if it was a hit check if its the closest
                if (hit.first)
                {
                    if ((closest_distance < 0.0f) || (hit.second < closest_distance))
                    {
                        // this is the closest so far, save it off
                        closest_distance = hit.second;
                        new_closest_found = true;
                    }
                }
            }

            // if we found a new closest raycast for this object, update the
            // closest_result before moving on to the next object.
            if (new_closest_found)
            {
                closest_result = ray.getPoint(closest_distance);
                (*result) = pentity;
            }
        }
    }

    // return the result
    if (closest_distance != max_distance)
    {
        hitpoint = closest_result;
        return (true);
    }
    else
    {
        // raycast failed
        return (false);
    }
}

//-------------------------------------------------------------------------------------
void Space::getMeshData(const Ogre::MeshPtr mesh, size_t &vertex_count, size_t &index_count,
                               const Ogre::Vector3 &position, const Ogre::Quaternion &orient, const Ogre::Vector3 &scale)
{
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;

    vertex_count = index_count = 0;

    // Calculate how many vertices and indices we're going to need
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh( i );

        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }

        // Add the indices
        index_count += submesh->indexData->indexCount;
    }


    if(vertex_count > mVertexBufferSize)
    {
        OGRE_FREE(mVertexBuffer, Ogre::MEMCATEGORY_GEOMETRY);
        mVertexBuffer = OGRE_ALLOC_T(Ogre::Vector3,vertex_count, Ogre::MEMCATEGORY_GEOMETRY);
        mVertexBufferSize = vertex_count;
    }

    if(index_count > mIndexBufferSize)
    {
        OGRE_FREE(mIndexBuffer, Ogre::MEMCATEGORY_GEOMETRY);
        mIndexBuffer = OGRE_ALLOC_T(unsigned long,index_count, Ogre::MEMCATEGORY_GEOMETRY);
        mIndexBufferSize = index_count;
    }

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);

        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
              added_shared = true;
              shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal;

            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);

                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

                mVertexBuffer[current_offset + j] = (orient * (pt * scale)) + position;
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }


        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                mIndexBuffer[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                mIndexBuffer[index_offset++] = static_cast<unsigned long>(pShort[k]) + static_cast<unsigned long>(offset);
            }
        }

        ibuf->unlock();
        current_offset = next_offset;
    }
    index_count = index_offset;
}

//-------------------------------------------------------------------------------------