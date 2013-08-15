
#ifndef __SPACEAVATARSELECT_CLIENT_h_
#define __SPACEAVATARSELECT_CLIENT_h_

#include "space.h"


class SpaceAvatarSelect : public Space
{
public:
    SpaceAvatarSelect(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr);
    virtual ~SpaceAvatarSelect(void);

    virtual void setupResources();
    virtual void createScene(void);
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual void buttonHit(OgreBites::Button* button);
	virtual void kbengine_onEvent(const KBEngine::EventData* lpEventData);
private:
};

#endif // #ifndef __SPACEAVATARSELECT_CLIENT_h_
