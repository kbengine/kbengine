#include "OgreApplication.h"
#include "DotSceneLoader.h"
#include "PagedGeometry.h"
#include "GrassLoader.h"
#include "BatchPage.h"
#include "ImpostorPage.h"
#include "TreeLoader3D.h"
#include "space_world.h"
#include "space_login.h"
#include "cstdkbe/cstdkbe.hpp"

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

Space* space = NULL;
//-------------------------------------------------------------------------------------
OgreApplication::OgreApplication(void)
{
}

//-------------------------------------------------------------------------------------
OgreApplication::~OgreApplication(void)
{
	mCameraMan = NULL;
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

    return true;
};

//-------------------------------------------------------------------------------------
void OgreApplication::setupResources(void)
{
    BaseApplication::setupResources();
}

//-------------------------------------------------------------------------------------
bool OgreApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    bool ret = BaseApplication::frameRenderingQueued(evt);
	
	kbe_update(); 

	if(space == NULL)
	{
		space = new SpaceLogin(mRoot, mWindow, mInputManager, mTrayMgr);
		space->setup();
	}
	else
	{
		space->frameRenderingQueued(evt);
	}

    return ret;
}

//-------------------------------------------------------------------------------------
bool OgreApplication::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

	if(space)
	{
		space->keyPressed(arg);
	}

    return BaseApplication::keyPressed( arg );
}

//-------------------------------------------------------------------------------------
void OgreApplication::buttonHit(OgreBites::Button* button)
{
	if(space)
	{
		space->buttonHit(button);
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
        OgreApplication app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

		kbe_destroy();

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		FreeLibrary(g_hKBEngineDll);
#endif

        return 0;
    }

#ifdef __cplusplus
}
#endif
