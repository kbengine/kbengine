#include "space.h"
#include "OgreApplication.h"

//-------------------------------------------------------------------------------------
Space::Space(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
	OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr):
mRoot(pOgreRoot),
mCamera(0),
mActiveCamera(0),
mSceneMgr(0),
mWindow(pRenderWin),
mInputManager(pInputMgr),
mTrayMgr(pTrayMgr),
mCameraMan(0)
{
	
}

//-------------------------------------------------------------------------------------
Space::~Space(void)
{
	if (mCameraMan) delete mCameraMan;
	if(mWindow) mWindow->removeAllViewports();
	if(mRoot && mSceneMgr)mRoot->destroySceneManager(mSceneMgr);
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