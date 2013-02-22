
#ifndef __SPACE_CLIENT_h_
#define __SPACE_CLIENT_h_

#include "BaseApplication.h"

class Space
{
public:
    Space(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr);

    virtual ~Space(void);

	virtual bool setup() = 0;
    virtual void setupResources() = 0;
    virtual void createScene(void) = 0;
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) = 0;

	virtual bool keyPressed( const OIS::KeyEvent &arg ){ return true; }
protected:
    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::Camera* mActiveCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;

	OIS::InputManager* mInputManager;
	OgreBites::SdkTrayManager*    mTrayMgr;
	OgreBites::SdkCameraMan* mCameraMan;
};

#endif // #ifndef __SPACE_CLIENT_h_
