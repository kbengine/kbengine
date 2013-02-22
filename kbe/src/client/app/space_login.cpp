#include "space_login.h"
#include "OgreApplication.h"

//-------------------------------------------------------------------------------------
SpaceLogin::SpaceLogin(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr)
:   Space(pOgreRoot, pRenderWin, pInputMgr, pTrayMgr)
{
}

//-------------------------------------------------------------------------------------
SpaceLogin::~SpaceLogin(void)
{
	mTrayMgr->removeWidgetFromTray("login");
}

//-------------------------------------------------------------------------------------
void SpaceLogin::setupResources(void)
{
}

//-------------------------------------------------------------------------------------
void SpaceLogin::createScene(void)
{
	mTrayMgr->createButton(OgreBites::TL_CENTER, "login", "login", 120);
	mTrayMgr->showCursor();

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
bool SpaceLogin::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	mTrayMgr->frameRenderingQueued(evt);
    return true;
}

//-------------------------------------------------------------------------------------
bool SpaceLogin::keyPressed( const OIS::KeyEvent &arg )
{
    return true;
}


