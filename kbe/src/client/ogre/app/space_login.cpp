#include "space_login.h"
#include "space_avatarselect.h"
#include "OgreApplication.h"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/stringconv.hpp"
#include "../../kbengine_dll/kbengine_dll.h"

Ogre::String g_accountName;

//-------------------------------------------------------------------------------------
SpaceLogin::SpaceLogin(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr)
:   Space(pOgreRoot, pRenderWin, pInputMgr, pTrayMgr)
{
}

//-------------------------------------------------------------------------------------
SpaceLogin::~SpaceLogin(void)
{
	mTrayMgr->destroyWidget("login");
	mTrayMgr->destroyWidget("accountName");
}

//-------------------------------------------------------------------------------------
void SpaceLogin::setupResources(void)
{
	// ogre²»ÏÔÊ¾ÎÄ×Öbug
	// http://www.ogre3d.org/forums/viewtopic.php?f=4&t=59197
	Ogre::ResourceManager::ResourceMapIterator iter = Ogre::FontManager::getSingleton().getResourceIterator();
	while (iter.hasMoreElements()) { 
		iter.getNext()->load(); 
	}
}

//-------------------------------------------------------------------------------------
void SpaceLogin::createScene(void)
{
	if(g_accountName.size() == 0)
		g_accountName = KBEngine::StringConv::val2str(KBEngine::genUUID64());

	mTrayMgr->createButton(OgreBites::TL_CENTER, "login", "fast login", 120);

	Ogre::StringVector values;
	values.push_back(g_accountName);
	mTrayMgr->createParamsPanel(OgreBites::TL_CENTER, "accountName", 300, values);

	mTrayMgr->showCursor();
	
	mTrayMgr->hideFrameStats();
	mTrayMgr->hideLogo();
	mTrayMgr->hideBackdrop();

	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

    // Create the camera
    mActiveCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mActiveCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mActiveCamera->lookAt(Ogre::Vector3(0,0,-300));
    mActiveCamera->setNearClipDistance(5);

    mCameraMan = new OgreBites::SdkCameraMan(mActiveCamera);   // create a default camera controller
    mCameraMan->setTopSpeed(7.0f);
	OgreApplication::getSingleton().setCurrCameraMan(mCameraMan);

    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mActiveCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mActiveCamera->setAspectRatio(
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

//-------------------------------------------------------------------------------------
void SpaceLogin::buttonHit(OgreBites::Button* button)
{
	if(button->getCaption() == "fast login")
	{
		if(!kbe_login(g_accountName.c_str(), "123456"))
		{
			MessageBox( NULL, "SpaceLogin::connect is error!", "warning!", MB_OK);
		}
	}
}

//-------------------------------------------------------------------------------------
void SpaceLogin::kbengine_onEvent(const KBEngine::EventData* lpEventData)
{
	switch(lpEventData->id)
	{
	case CLIENT_EVENT_LOGIN_SUCCESS:
		break;
	case CLIENT_EVENT_LOGIN_FAILED:
		MessageBox( NULL, "SpaceLogin::kbengine_onEvent login is failed!", "warning!", MB_OK);
		break;
	case CLIENT_EVENT_LOGIN_GATEWAY_SUCCESS:
		OgreApplication::getSingleton().changeSpace(new SpaceAvatarSelect(mRoot, mWindow, mInputManager, mTrayMgr));
		break;
	case CLIENT_EVENT_LOGIN_GATEWAY_FAILED:
		MessageBox( NULL, "SpaceLogin::kbengine_onEvent loginGateway is failed!", "warning!", MB_OK);
		break;
	default:
		break;
	};
}