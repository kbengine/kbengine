
#ifndef __SPACE_CLIENT_h_
#define __SPACE_CLIENT_h_

#include "BaseApplication.h"
#include "client_lib/event.hpp"

class Space
{
public:
    Space(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr);

    virtual ~Space(void);

	virtual bool setup();
    virtual void setupResources() = 0;
    virtual void createScene(void) = 0;
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) = 0;

	virtual bool keyPressed(const OIS::KeyEvent &arg){ return true; }
	virtual bool keyReleased(const OIS::KeyEvent &arg){ return true; }
	virtual void buttonHit(OgreBites::Button* button){}

	virtual void kbengine_onEvent(const KBEngine::EventData* lpEventData){};
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
