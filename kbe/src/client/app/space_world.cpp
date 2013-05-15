#include "space_world.h"
#include "space_login.h"
#include "DotSceneLoader.h"
#include "PagedGeometry.h"
#include "GrassLoader.h"
#include "BatchPage.h"
#include "ImpostorPage.h"
#include "TreeLoader3D.h"
#include "OgreApplication.h"

#include "../kbengine_dll/kbengine_dll.h"

// 记录选择
KBEngine::ENTITY_ID g_tickSelTargetID = -1;

//-------------------------------------------------------------------------------------
SpaceWorld::SpaceWorld(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr)
:   Space(pOgreRoot, pRenderWin, pInputMgr, pTrayMgr),
	mLoader(0),
    mTerrainImported(true),
    mSceneFile(Ogre::StringUtil::BLANK),
    mHelpInfo(Ogre::StringUtil::BLANK),
    mFly(false),
	mPlayerPtr(0),
	mTargetPtr(0),
	mEntities(),
	serverClosed_(false),
	showCloseButton_(false),
	pDecalObj_(NULL),
	pSelDecalObj_(NULL)
{
    mHelpInfo = Ogre::String("Use [W][A][S][D] keys for movement.\nKeys [1]-[9] to switch between cameras.\n[0] toggles SceneNode debug visuals.\n\nPress [C] to toggle clamp to terrain (gravity).\n\n[G] toggles the detail panel.\n[R] cycles polygonModes (Solid/Wireframe/Points).\n[T] cycles various filtering.\n\n\nPress [ESC] to quit.");
}

//-------------------------------------------------------------------------------------
SpaceWorld::~SpaceWorld(void)
{
	if(showCloseButton_)
		mTrayMgr->destroyWidget("backlogin");

	mSceneMgr->destroyCamera("mainCamera");

	mActiveCamera = NULL;
	delete mLoader;
	
	mTargetPtr = mPlayerPtr = NULL;
	mEntities.clear();
}

//-------------------------------------------------------------------------------------
void SpaceWorld::setupResources(void)
{
    rapidxml::xml_document<> XMLDoc;    // character type defaults to char
    rapidxml::xml_node<>* XMLRoot;

    std::ifstream fp;
    fp.open("SampleAppConfig.xml", std::ios::in | std::ios::binary);
    Ogre::DataStreamPtr stream(OGRE_NEW Ogre::FileStreamDataStream("SampleAppConfig.xml", &fp, false));
    char* sampleAppConfig = strdup(stream->getAsString().c_str());
    XMLDoc.parse<0>(sampleAppConfig);
    XMLRoot = XMLDoc.first_node("SampleApp");

    Ogre::String projectDir = Ogre::String(XMLRoot->first_attribute("projectDir")->value());
    mSceneFile = Ogre::String(XMLRoot->first_attribute("scene")->value());

    // add sample project directory to the resource paths
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir, "FileSystem", "Space");

    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Materials", "FileSystem", "Space");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Models", "FileSystem", "Space");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Terrain", "FileSystem", "Space");

	mTrayMgr->showLoadingBar(1, 0);
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Space");
	mTrayMgr->hideLoadingBar();
}

//-------------------------------------------------------------------------------------
void SpaceWorld::createScene(void)
{
	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

    mLoader = new DotSceneLoader(SCENE_MASK);
    mLoader->parseDotScene(mSceneFile, "Space", mSceneMgr);

	mTrayMgr->showCursor();
	mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	mTrayMgr->showBackdrop();
	mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);

	// set shadow properties
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	mActiveCamera = mSceneMgr->createCamera("mainCamera");
	// Create one viewport, entire window
	Ogre::Viewport* vp = mWindow->addViewport(mActiveCamera);
	vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

	// Alter the camera aspect ratio to match the viewport
	mActiveCamera->setAspectRatio(
		Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

    mWindow->getViewport(0)->setCamera(mActiveCamera);

	mActiveCamera->setNearClipDistance(0.1f);
	mActiveCamera->setFarClipDistance(30000);

	mCameraMan = new OgreBites::SdkCameraMan(mActiveCamera);   // create a default camera controller
	mCameraMan->setTopSpeed(7.0f);
    mCameraMan->setCamera(mActiveCamera);
	OgreApplication::getSingleton().setCurrCameraMan(mCameraMan);


	mCameraMan->setStyle(OgreBites::CS_MANUAL);

	mRaySceneQuery = mSceneMgr->createRayQuery(Ray());
	pDecalObj_ = createDecal("Examples/Decal");
	pSelDecalObj_ = createDecal("Examples/Sel_Decal");
}

//----------------------------------------------------------------------------------------
Ogre::ManualObject* SpaceWorld::createDecal(Ogre::String decalMaterialName)
{
	Ogre::ManualObject* pDecalObj = new Ogre::ManualObject(decalMaterialName);
    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pDecalObj);
   // pDecalObj->setCastShadows(false);
	//pDecalObj->setDynamic(true);  
	//pDecalObj->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_2);  

	int x_size = 64;  // number of polygons
	int z_size = 64;
    pDecalObj->begin(decalMaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);

    for (int i=0; i<x_size; i++)
    {
        for (int j=0; j<=z_size; j++)
        {
            pDecalObj->position(Ogre::Vector3(i, 0, j));
            pDecalObj->textureCoord((float)i / (float)x_size, (float)j / (float)z_size);
        }
    }

    for (int i=0; i<x_size; i++)
    {
        for (int j=0; j<z_size; j++)
        {
            pDecalObj->quad( i * (x_size+1) + j, i * (x_size+1) + j + 1,
                (i + 1) * (x_size+1) + j + 1,(i + 1) * (x_size+1) + j);
        }
    }

    pDecalObj->end();

	return pDecalObj;
}

//----------------------------------------------------------------------------------------
void SpaceWorld::moveDecalTo(Ogre::ManualObject* pDecal, const Ogre::Vector3& position, float radius)
{
	Ogre::Real x = position.x;
	Ogre::Real z = position.z;

    Ogre::Real x1 = x - radius;
    Ogre::Real z1 = z - radius;

	int x_size = 64;  // number of polygons
	int z_size = 64;

    Ogre::Real x_step = radius * 2/ x_size;
    Ogre::Real z_step = radius * 2/ z_size;

    pDecal->beginUpdate(0);

    for (int i=0; i<=x_size; i++)
    {
        for (int j=0; j<=z_size; j++)
        {    
			pDecal->position(Ogre::Vector3(x1, getPositionHeight(Ogre::Vector3(x1, 0, z1)) + 0.1, z1));
            pDecal->textureCoord((float)i / (float)x_size, (float)j / (float)z_size);

            z1 += z_step;
        }

        x1 += x_step;

        z1 = z - radius;
    }

    for (int i=0; i<x_size; i++)
    {
        for (int j=0; j<z_size; j++)
        {
            pDecal->quad( i * (x_size+1) + j, i * (x_size+1) + j + 1, 
                (i + 1) * (x_size+1) + j + 1, (i + 1) * (x_size+1) + j);
        }
    }

    pDecal->end();
}

//----------------------------------------------------------------------------------------
float SpaceWorld::getPositionHeight(const Ogre::Vector3& pos)
{ 
	return mLoader->getTerrainGroup()->getHeightAtWorldPosition(pos); 
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if (!mLoader->getTerrainGroup()->isDerivedDataUpdateInProgress())
    {
        if (mTerrainImported)
        {
            mLoader->getTerrainGroup()->saveAllTerrains(true);
            mTerrainImported = false;
        }
    }

    for(unsigned int ij = 0;ij < mLoader->mPGHandles.size();ij++)
    {
        mLoader->mPGHandles[ij]->update();
    }
	
	ENTITIES::iterator iter = mEntities.begin();
	for(; iter != mEntities.end(); iter++)
	{
		iter->second->addTime(evt.timeSinceLastFrame);
	}
	
	if(mPlayerPtr) 
	{
		// kbe_lock();

		kbe_updateVolatile(g_tickSelTargetID, 
							mPlayerPtr->getPosition().x, 
							mPlayerPtr->getPosition().y, 
							mPlayerPtr->getPosition().z,
							mPlayerPtr->getDirection().x, 
							mPlayerPtr->getDirection().y, 
							mPlayerPtr->getDirection().z);

		g_tickSelTargetID = -1;
		// kbe_unlock();
	}
	
	// 服务器关闭
	if(serverClosed_)
	{
		mTrayMgr->createButton(OgreBites::TL_CENTER, "backlogin", "back login", 120);
		serverClosed_ = false;
		showCloseButton_ = true;
	}

    return true;
}

//-------------------------------------------------------------------------------------
void SpaceWorld::buttonHit(OgreBites::Button* button)
{
	if(button->getCaption() == "back login" && showCloseButton_)
	{
		OgreApplication::getSingleton().changeSpace(new SpaceLogin(mRoot, mWindow, mInputManager, mTrayMgr));
	}
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::keyPressed( const OIS::KeyEvent &arg )
{
    if (arg.key == OIS::KC_H || arg.key == OIS::KC_F1)   // toggle visibility of help dialog
    {
        if (!mTrayMgr->isDialogVisible()) mTrayMgr->showOkDialog("Help", mHelpInfo);
        else mTrayMgr->closeDialog();
    }
    if (arg.key == OIS::KC_0)   // toggle scenenode debug renderables
    {
        mSceneMgr->setDisplaySceneNodes(!mSceneMgr->getDisplaySceneNodes());
    }
    if (arg.key == OIS::KC_C)   // toggle fly/walk
    {
        mFly = !mFly;
    }

	if(mPlayerPtr) mPlayerPtr->injectKeyDown(arg);
    return true; 
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::keyReleased(const OIS::KeyEvent &arg)
{
	if(mPlayerPtr) mPlayerPtr->injectKeyUp(arg);
    return true;
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::mouseMoved( const OIS::MouseEvent &arg )
{
    if(mPlayerPtr) mPlayerPtr->injectMouseMove(arg);
    return false;
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if(mPlayerPtr) mPlayerPtr->injectMouseDown(arg, id);

	if(id == OIS::MB_Left)
	{
		Ogre::Ray mouseRay = mActiveCamera->getCameraToViewportRay(arg.state.X.abs / float(arg.state.width),
			arg.state.Y.abs / float(arg.state.height));

		Ogre::Entity* rayResult = NULL;
		Ogre::Vector3 hitPoint;
		if(pickEntity(mRaySceneQuery, mouseRay, &rayResult, ENTITY_MASK, 0, hitPoint, "x", 5000))
		{
			Ogre::String key = rayResult->getName();
			Ogre::StringVector vec = Ogre::StringUtil::split(key, "_");
			if(vec.size() >= 2)
			{
				key = vec[0];
				KBEngine::ENTITY_ID eid = Ogre::StringConverter::parseInt(key);

				ENTITIES::iterator iter = mEntities.find(eid);
				if(iter != mEntities.end())
				{
					g_tickSelTargetID = eid;
					if(mTargetPtr != iter->second.get())
					{
						mTargetPtr = iter->second.get();
						moveDecalTo(pSelDecalObj_, rayResult->getParentSceneNode()->getPosition(), 1.5f);
					}
					else
					{
						// 再次选择了该entity
					}
				}
			}
		}
		else
		{
			Ogre::Ray ray = mTrayMgr->getCursorRay(mActiveCamera);
			Ogre::TerrainGroup::RayResult rayResult = mLoader->getTerrainGroup()->rayIntersects(ray);
			if (rayResult.hit)
			{
				/*
				if(m_player_ != NULL)
				{
					m_player_->moveTo(target, m_player_->getSpeed());
					KBEngine::onMousePressedInWorld(target.x, target.y, target.z);
				}
				*/

				moveDecalTo(pDecalObj_, rayResult.position, 1.5f);
			}
		}

	}
    return false;
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    return true;
}

//-------------------------------------------------------------------------------------
void SpaceWorld::kbengine_onEvent(const KBEngine::EventData* lpEventData)
{
	switch(lpEventData->id)
	{
	case CLIENT_EVENT_CREATEDENTITY:
		{
			const KBEngine::EventData_CreatedEntity* pEventData_createEntity = static_cast<const KBEngine::EventData_CreatedEntity*>(lpEventData);
			KBEngine::ENTITY_ID eid = pEventData_createEntity->pEntity->aspectID();
			KBEntity* pEntity = new KBEntity(this, eid);
			mEntities[eid].reset(pEntity);
			//pEntity->visable(false);
		}
		break;
	case CLIENT_EVENT_ENTERWORLD:
		{
			const KBEngine::EventData_EnterWorld* pEventData_EnterWorld = static_cast<const KBEngine::EventData_EnterWorld*>(lpEventData);
			KBEngine::ENTITY_ID eid = pEventData_EnterWorld->pEntity->aspectID();
			
			ENTITIES::iterator iter = mEntities.find(eid);
			if(iter == mEntities.end())
				break;
			
			KBEntity* pEntity = iter->second.get();

			pEntity->setupBody(mSceneMgr);
			pEntity->setupAnimations();

			pEntity->setPosition(pEventData_EnterWorld->x, pEventData_EnterWorld->y, pEventData_EnterWorld->z);
			pEntity->setDestPosition(pEventData_EnterWorld->x, pEventData_EnterWorld->y, pEventData_EnterWorld->z);
			pEntity->scale(0.3f, 0.3f, 0.3f);
			pEntity->setMoveSpeed(pEventData_EnterWorld->speed);
			pEntity->setDestDirection(pEventData_EnterWorld->yaw, pEventData_EnterWorld->pitch, pEventData_EnterWorld->roll);
			pEntity->setDirection(pEventData_EnterWorld->yaw, pEventData_EnterWorld->pitch, pEventData_EnterWorld->roll);

			if(kbe_playerID() == eid)
			{
				pEntity->setupCamera(mActiveCamera);
				mPlayerPtr = pEntity;
			}
			
			pEntity->inWorld(true);

			//pEntity->visable(true);
		}
		break;
	case CLIENT_EVENT_LEAVEWORLD:
		{
			KBEngine::ENTITY_ID eid = static_cast<const KBEngine::EventData_EnterWorld*>(lpEventData)->pEntity->aspectID();
			if(kbe_playerID() == eid)
				mPlayerPtr = NULL;
			
			if(mTargetPtr && mTargetPtr->id() == eid)
				mTargetPtr = NULL;

			mEntities.erase(eid);
		}
		break;
	case CLIENT_EVENT_POSITION_CHANGED:
		{
			const KBEngine::EventData_PositionChanged* pEventData = static_cast<const KBEngine::EventData_PositionChanged*>(lpEventData);
			KBEngine::ENTITY_ID eid = pEventData->pEntity->aspectID();

			ENTITIES::iterator iter = mEntities.find(eid);
			if(iter == mEntities.end())
				break;
			
			iter->second->setDestPosition(pEventData->x, pEventData->y, pEventData->z);
		}
		break;
	case CLIENT_EVENT_DIRECTION_CHANGED:
		{
			const KBEngine::EventData_DirectionChanged* pEventData = static_cast<const KBEngine::EventData_DirectionChanged*>(lpEventData);
			KBEngine::ENTITY_ID eid = pEventData->pEntity->aspectID();

			ENTITIES::iterator iter = mEntities.find(eid);
			if(iter == mEntities.end())
				break;

			iter->second->setDestDirection(pEventData->yaw, pEventData->pitch, pEventData->roll);
		}
		break;
	case CLIENT_EVENT_MOVESPEED_CHANGED:
		{
			const KBEngine::EventData_MoveSpeedChanged* pEventData = static_cast<const KBEngine::EventData_MoveSpeedChanged*>(lpEventData);
			KBEngine::ENTITY_ID eid = pEventData->pEntity->aspectID();
			
			ENTITIES::iterator iter = mEntities.find(eid);
			if(iter == mEntities.end())
				break;

			iter->second->setMoveSpeed(pEventData->speed);
		}
		break;
	case CLIENT_EVENT_SERVER_CLOSED:
		{
			serverClosed_ = true;
		}
		break;
		case CLIENT_EVENT_SCRIPT:
			{
				const KBEngine::EventData_Script* peventdata = static_cast<const KBEngine::EventData_Script*>(lpEventData);
				if(peventdata->name == "set_name")
				{
					if(peventdata->argsSize > 0)
					{
						PyObject* pyitem0 = PyTuple_GetItem(peventdata->pyDatas, 0);
						PyObject* pyitem1 = PyTuple_GetItem(peventdata->pyDatas, 1);
						
						KBEngine::ENTITY_ID eid = PyLong_AsUnsignedLong(pyitem0);

						ENTITIES::iterator iter = mEntities.find(eid);
						if(iter == mEntities.end())
							break;
						
						KBEntity* pEntity = iter->second.get();

						wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyitem1, NULL);
						pEntity->setName(PyUnicode_AsWideCharStringRet0);
						//char* name = wchar2char(PyUnicode_AsWideCharStringRet0);
						PyMem_Free(PyUnicode_AsWideCharStringRet0);
						//free(name);																				
					}
				}
				else if(peventdata->name == "set_modelScale")
				{
					if(peventdata->argsSize > 0)
					{
						PyObject* pyitem0 = PyTuple_GetItem(peventdata->pyDatas, 0);
						PyObject* pyitem1 = PyTuple_GetItem(peventdata->pyDatas, 1);
						
						KBEngine::ENTITY_ID eid = PyLong_AsUnsignedLong(pyitem0);
						uint32 scale = PyLong_AsUnsignedLong(pyitem1);	

						ENTITIES::iterator iter = mEntities.find(eid);
						if(iter == mEntities.end())
							break;

						iter->second->scale(scale / 100.0);
					}
				}
				else if(peventdata->name == "set_state")
				{
					if(peventdata->argsSize > 0)
					{
						PyObject* pyitem0 = PyTuple_GetItem(peventdata->pyDatas, 0);
						PyObject* pyitem1 = PyTuple_GetItem(peventdata->pyDatas, 1);
						
						KBEngine::ENTITY_ID eid = PyLong_AsUnsignedLong(pyitem0);
						int32 state = PyLong_AsLong(pyitem1);	

						ENTITIES::iterator iter = mEntities.find(eid);
						if(iter == mEntities.end())
							break;

						iter->second->setState(state);
					}
				}
				else if(peventdata->name == "set_HP_Max")
				{
					if(peventdata->argsSize > 0)
					{
						PyObject* pyitem0 = PyTuple_GetItem(peventdata->pyDatas, 0);
						PyObject* pyitem1 = PyTuple_GetItem(peventdata->pyDatas, 1);
						
						KBEngine::ENTITY_ID eid = PyLong_AsUnsignedLong(pyitem0);
						int32 v = PyLong_AsLong(pyitem1);	

						ENTITIES::iterator iter = mEntities.find(eid);
						if(iter == mEntities.end())
							break;

						iter->second->setHPMAX(v);
					}
				}
				else if(peventdata->name == "set_MP_Max")
				{
					if(peventdata->argsSize > 0)
					{
						PyObject* pyitem0 = PyTuple_GetItem(peventdata->pyDatas, 0);
						PyObject* pyitem1 = PyTuple_GetItem(peventdata->pyDatas, 1);
						
						KBEngine::ENTITY_ID eid = PyLong_AsUnsignedLong(pyitem0);
						int32 v = PyLong_AsLong(pyitem1);	

						ENTITIES::iterator iter = mEntities.find(eid);
						if(iter == mEntities.end())
							break;

						iter->second->setMPMAX(v);
					}
				}
				else if(peventdata->name == "recvDamage")
				{
					if(peventdata->argsSize > 0)
					{
						PyObject* pyitem0 = PyTuple_GetItem(peventdata->pyDatas, 0);
						PyObject* pyitem1 = PyTuple_GetItem(peventdata->pyDatas, 1);
						PyObject* pyitem2 = PyTuple_GetItem(peventdata->pyDatas, 2);
						PyObject* pyitem3 = PyTuple_GetItem(peventdata->pyDatas, 3);
						PyObject* pyitem4 = PyTuple_GetItem(peventdata->pyDatas, 4);

						KBEngine::ENTITY_ID eid = PyLong_AsUnsignedLong(pyitem0);
						KBEngine::ENTITY_ID attackerID = PyLong_AsUnsignedLong(pyitem1);	
						uint32 skillID = PyLong_AsUnsignedLong(pyitem2);	
						uint32 damageType = PyLong_AsUnsignedLong(pyitem3);	
						uint32 damage = PyLong_AsUnsignedLong(pyitem4);	

						ENTITIES::iterator iter = mEntities.find(attackerID);
						ENTITIES::iterator iter1 = mEntities.find(eid);
						
						KBEntity* attacker = NULL;
						KBEntity* receiver = NULL;

						if(iter != mEntities.end())
						{
							attacker = iter->second.get();
						}

						if(iter1 != mEntities.end())
						{
							receiver = iter1->second.get();
						}

						if(attacker)
						{
							attacker->attack(receiver, skillID, damageType, damage);
						}

						if(receiver)
						{
							receiver->recvDamage(attacker, skillID, damageType, damage);
						}
					}
				}
			}
			break;
	default:
		break;
	};
}
