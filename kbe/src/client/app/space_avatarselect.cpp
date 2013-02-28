#include "space_avatarselect.h"
#include "OgreApplication.h"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/stringconv.hpp"
#include "../kbengine_dll/kbengine_dll.h"

//-------------------------------------------------------------------------------------
SpaceAvatarSelect::SpaceAvatarSelect(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr)
:   Space(pOgreRoot, pRenderWin, pInputMgr, pTrayMgr)
{
}

//-------------------------------------------------------------------------------------
SpaceAvatarSelect::~SpaceAvatarSelect(void)
{
	mTrayMgr->destroyWidget("start");
	mTrayMgr->destroyWidget("create");
}

//-------------------------------------------------------------------------------------
void SpaceAvatarSelect::setupResources(void)
{
	// ogre²»ÏÔÊ¾ÎÄ×Öbug
	// http://www.ogre3d.org/forums/viewtopic.php?f=4&t=59197
	Ogre::ResourceManager::ResourceMapIterator iter = Ogre::FontManager::getSingleton().getResourceIterator();
	while (iter.hasMoreElements()) { 
		iter.getNext()->load(); 
	}
}

//-------------------------------------------------------------------------------------
void SpaceAvatarSelect::createScene(void)
{
	mTrayMgr->createButton(OgreBites::TL_BOTTOMRIGHT, "start", "start", 120);
	mTrayMgr->createButton(OgreBites::TL_BOTTOMLEFT, "create", "create avatar", 120);
	mTrayMgr->showCursor();
	
	mTrayMgr->hideFrameStats();
	mTrayMgr->hideLogo();
	mTrayMgr->hideBackdrop();

	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
    mCameraMan->setTopSpeed(7.0f);
	OgreApplication::getSingleton().setCurrCameraMan(mCameraMan);

    mActiveCamera = mCamera;

    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}

//-------------------------------------------------------------------------------------
bool SpaceAvatarSelect::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	mTrayMgr->frameRenderingQueued(evt);
    return true;
}

//-------------------------------------------------------------------------------------
bool SpaceAvatarSelect::keyPressed( const OIS::KeyEvent &arg )
{
    return true;
}

//-------------------------------------------------------------------------------------
void SpaceAvatarSelect::buttonHit(OgreBites::Button* button)
{
	if(button->getCaption() == "start")
	{
	}
	else if(button->getCaption() == "create avatar")
	{
		PyObject* args = Py_BuildValue("is", 1, "kbengine");
		PyObject* ret = kbe_callEntityMethod(kbe_playerID(), "reqCreateAvatar", args);
		Py_DECREF(args);
		Py_DECREF(ret);
	}
}

//-------------------------------------------------------------------------------------
void SpaceAvatarSelect::kbengine_onEvent(const KBEngine::EventData* lpEventData)
{
}