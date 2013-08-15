
#ifndef __SPACELOGIN_CLIENT_h_
#define __SPACELOGIN_CLIENT_h_

#include "space.h"


class SpaceLogin : public Space
{
public:
    SpaceLogin(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr);
    virtual ~SpaceLogin(void);

    virtual void setupResources();
    virtual void createScene(void);
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual void buttonHit(OgreBites::Button* button);
	virtual void kbengine_onEvent(const KBEngine::EventData* lpEventData);
private:
};

#endif // #ifndef __SPACELOGIN_CLIENT_h_
