#include "OgreApplication.h"
#include "DotSceneLoader.h"
#include "PagedGeometry.h"
#include "GrassLoader.h"
#include "BatchPage.h"
#include "ImpostorPage.h"
#include "TreeLoader3D.h"
#include "space_world.h"
#include "space_login.h"
#include "space_avatarselect.h"
#include "cstdkbe/cstdkbe.hpp"
#include "pyscript/pythread_lock.hpp"

#include "../kbengine_dll/kbengine_dll.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
HINSTANCE g_hKBEngineDll = NULL;
#endif

namespace KBEngine{
	COMPONENT_TYPE g_componentType = CLIENT_TYPE;
	COMPONENT_ID g_componentID = 1;
	COMPONENT_ORDER g_componentOrder = 1;
}

template<> OgreApplication* Ogre::Singleton<OgreApplication>::msSingleton = 0;

Space* g_space = NULL;
boost::mutex g_spaceMutex;
volatile bool g_hasEvent = false;

//-------------------------------------------------------------------------------------
OgreApplication::OgreApplication(void):
events_()
{
	kbe_registerEventHandle(this);
}

//-------------------------------------------------------------------------------------
OgreApplication::~OgreApplication(void)
{
	kbe_deregisterEventHandle(this);
	mCameraMan = NULL;
	SAFE_RELEASE(g_space);
}

//-------------------------------------------------------------------------------------
void OgreApplication::go(void)
{
    if (!setup())
        return;

	while(!mShutDown)
	{
		mRoot->renderOneFrame();

		// 通知系统分派消息
		Ogre::WindowEventUtilities::messagePump();

		if(!g_hasEvent)
			Sleep(1);
	}

    // clean up
    destroyScene();
}

//-------------------------------------------------------------------------------------
bool OgreApplication::setup(void)
{
#ifdef _DEBUG
    mRoot = new Ogre::Root("plugins_d.cfg");
#else
    mRoot = new Ogre::Root();
#endif
    setupResources();

    if (!configure()) return false;

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();

    createFrameListener();

	changeSpace(new SpaceLogin(mRoot, mWindow, mInputManager, mTrayMgr));
    return true;
};

//-------------------------------------------------------------------------------------
void OgreApplication::changeSpace(Space* space)
{
	if(g_space)
		delete g_space;

	g_space = space;
	space->setup();
}

//-------------------------------------------------------------------------------------
void OgreApplication::setupResources(void)
{
    BaseApplication::setupResources();
}

//-------------------------------------------------------------------------------------
bool OgreApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if(g_space == NULL)
	{
		return BaseApplication::frameRenderingQueued(evt);
	}
	
	while(true)
	{
		g_spaceMutex.lock();

		if(events_.empty())
		{
			g_spaceMutex.unlock();
			break;
		}

		std::tr1::shared_ptr<const KBEngine::EventData> pEventData = events_.front();
		events_.pop();
		g_spaceMutex.unlock();

		KBEngine::EventID id = pEventData->id;
		
		if(id == CLIENT_EVENT_SERVER_CLOSED)
		{
			//OgreApplication::getSingleton().changeSpace(new SpaceAvatarSelect(mRoot, mWindow, mInputManager, mTrayMgr));
			//break;
		}

		// 如果需要在本线程访问脚本层则需要锁住引擎
		if(id == CLIENT_EVENT_SCRIPT)
		{
			kbe_lock();
		}

		g_space->kbengine_onEvent(pEventData.get());

		if(id == CLIENT_EVENT_SCRIPT)
		{
			kbe_unlock();
		}

		g_hasEvent = false;
	}

	if(!g_space->frameRenderingQueued(evt))
		return true;

    return BaseApplication::frameRenderingQueued(evt);
}

//-------------------------------------------------------------------------------------
bool OgreApplication::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

	if(g_space)
	{
		if(!g_space->keyPressed(arg))
			return true;
	}

    return BaseApplication::keyPressed( arg );
}

//-------------------------------------------------------------------------------------
bool OgreApplication::keyReleased( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

	if(g_space)
	{
		if(!g_space->keyReleased(arg))
			return true;
	}

    return BaseApplication::keyReleased(arg);
}

//-------------------------------------------------------------------------------------
bool OgreApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    mTrayMgr->injectMouseMove(arg);
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if(mCameraMan)
       mCameraMan->injectMouseMove(arg);

	if(g_space)
	{
		if(!g_space->mouseMoved(arg))
			return true;
	}

    return true;
}

//-------------------------------------------------------------------------------------
bool OgreApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    mTrayMgr->injectMouseDown(arg, id);
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if(mCameraMan)
       mCameraMan->injectMouseDown(arg, id);

	if(g_space)
	{
		if(!g_space->mousePressed(arg, id))
			return true;
	}

    return true;
}

//-------------------------------------------------------------------------------------
bool OgreApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    mTrayMgr->injectMouseUp(arg, id);
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if(mCameraMan)
       mCameraMan->injectMouseUp(arg, id);

	if(g_space)
	{
		if(!g_space->mouseReleased(arg, id))
			return true;
	}

    return true;
}

//-------------------------------------------------------------------------------------
void OgreApplication::buttonHit(OgreBites::Button* button)
{
	if(g_space)
	{
		g_space->buttonHit(button);
	}
}

//-------------------------------------------------------------------------------------
void OgreApplication::kbengine_onEvent(const KBEngine::EventData* lpEventData)
{
	KBEngine::EventData* peventdata = KBEngine::copyKBEngineEvent(lpEventData);

	if(peventdata)
	{
		boost::mutex::scoped_lock lock(g_spaceMutex);
		events_.push(std::tr1::shared_ptr<const KBEngine::EventData>(peventdata));
		g_hasEvent = true;
	}
}

//-------------------------------------------------------------------------------------
bool OgreApplication::PickEntity(Ogre::RaySceneQuery* mRaySceneQuery, Ogre::Ray &ray, Ogre::Entity **result, Ogre::uint32 mask ,Ogre::Vector3 &hitpoint, bool excludeInVisible,const Ogre::String& excludeobject, Ogre::Real max_distance)
{
	mRaySceneQuery->setRay(ray);
	mRaySceneQuery->setQueryMask(mask);
	mRaySceneQuery->setQueryTypeMask(Ogre::SceneManager::ENTITY_TYPE_MASK);
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
		{
			break;
		}

		// only check this result if its a hit against an entity
		if ((query_result[qr_idx].movable != NULL) && (query_result[qr_idx].movable->getMovableType().compare("Entity") == 0))
		{
			// get the entity to check
			Ogre::Entity *pentity = static_cast<Ogre::Entity*>(query_result[qr_idx].movable);

			if(excludeInVisible)
				if (!pentity->getVisible())
					continue;
			if(pentity->getName() == excludeobject)
				continue;

			// mesh data to retrieve
			size_t vertex_count;
			size_t index_count;
			Ogre::Vector3 *vertices;
			unsigned long *indices;

			// get the mesh information
			GetMeshInformationEx(pentity->getMesh(), vertex_count, vertices, index_count, indices,
				pentity->getParentNode()->_getDerivedPosition(),
				pentity->getParentNode()->_getDerivedOrientation(),
				pentity->getParentNode()->_getDerivedScale());

			//maybe there is a bug in GetMeshInformationEx(),when mesh is a line or a circle, the vertex_count is not multiple of 3
			//            if (index_count%3 != 0)
			//            {
			//                index_count-=index_count%3;
			//            }

			// test for hitting individual triangles on the mesh
			bool new_closest_found = false;
			for (int i = 0; i < static_cast<int>(index_count); i += 3)
			{

				// check for a hit against this triangle
				std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, vertices[indices[i]],
					vertices[indices[i+1]], vertices[indices[i+2]], true, true);

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

			// free the verticies and indicies memory
			delete[] vertices;
			delete[] indices;

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
		return true;
	}
	else
	{
		// raycast failed
		return false;
	}
}

//-------------------------------------------------------------------------------------
void OgreApplication::GetMeshInformationEx(const Ogre::MeshPtr mesh,
	size_t &vertex_count,
	Ogre::Vector3* &vertices,
	size_t &index_count,
	unsigned long* &indices,
	const Ogre::Vector3 &position,
	const Ogre::Quaternion &orient,
	const Ogre::Vector3 &scale)
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


	// Allocate space for the vertices and indices
	vertices = new Ogre::Vector3[vertex_count];
	indices = new unsigned long[index_count];

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

				vertices[current_offset + j] = (orient * (pt * scale)) + position;
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
				indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
			}
		}
		else
		{
			for ( size_t k = 0; k < numTris*3; ++k)
			{
				indices[index_offset++] = static_cast<unsigned long>(pShort[k]) + static_cast<unsigned long>(offset);
			}
		}

		ibuf->unlock();
		current_offset = next_offset;
	}
}

//-------------------------------------------------------------------------------------
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#ifdef _DEBUG
		std::string kbenginedll_name = "kbengine_d.dll";
#else
		std::string kbenginedll_name = "kbengine.dll";
#endif

		g_hKBEngineDll = LoadLibrary(kbenginedll_name.c_str());
		if (g_hKBEngineDll == NULL)
		{
			std::string kbenginedll_name_failed = "load " + kbenginedll_name + " is failed!";
			MessageBox( NULL, kbenginedll_name_failed.c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
			return 0;
		}
#endif

		if(!kbe_init())
		{
			MessageBox( NULL, "kbengine_init() is failed!", "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
			return 0;
		}

        // Create application object
        OgreApplication* app = new OgreApplication();

        try {
            app->go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }
		
		delete app;
		kbe_destroy();

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		FreeLibrary(g_hKBEngineDll);
#endif

        return 0;
    }

#ifdef __cplusplus
}
#endif
