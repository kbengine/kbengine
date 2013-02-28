#include "space_avatarselect.h"
#include "OgreApplication.h"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/stringconv.hpp"
#include "pyscript/pythread_lock.hpp"
#include "../kbengine_dll/kbengine_dll.h"

std::vector<Ogre::String> g_avatars;

char* wchar2char(const wchar_t* ts)
{
	int len = (wcslen(ts) + 1) * 4;
	char* ccattr =(char *)malloc(len);
	memset(ccattr, 0, len);
	wcstombs(ccattr, ts, len);
	return ccattr;
};

wchar_t* char2wchar(const char* cs)
{
	int len = (strlen(cs) + 1) * 4;
	wchar_t* ccattr =(wchar_t *)malloc(len);
	memset(ccattr, 0, len);
	mbstowcs(ccattr, cs, len);
	return ccattr;
};

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

	for(KBEngine::uint32 i=0; i<g_avatars.size(); i++)
	{
		mTrayMgr->destroyWidget(g_avatars[i]);
	}
	
	g_avatars.clear();
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
	
	/*
	mTrayMgr->createButton(OgreBites::TL_CENTER, "avatar1", "avatar", 300);
	mTrayMgr->createButton(OgreBites::TL_CENTER, "avatar2", "avatar", 300);
	mTrayMgr->createButton(OgreBites::TL_CENTER, "avatar3", "avatar", 300);
	*/

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
		Py_XDECREF(ret);
	}
}

//-------------------------------------------------------------------------------------
void SpaceAvatarSelect::kbengine_onEvent(const KBEngine::EventData* lpEventData)
{
	switch(lpEventData->id)
	{
		case CLIENT_EVENT_SCRIPT:
			{
				const KBEngine::EventData_Script* peventdata = static_cast<const KBEngine::EventData_Script*>(lpEventData);
				if(peventdata->name == "update_avatars")
				{
					if(peventdata->argsSize > 0)
					{
						PyObject* pyitem = PyTuple_GetItem(peventdata->pyDatas, 0);
						for(KBEngine::uint32 i=0; i<g_avatars.size(); i++)
						{
							mTrayMgr->destroyWidget(g_avatars[i]);
						}
						
						g_avatars.clear();

						PyObject *key, *value;
						Py_ssize_t pos = 0;
						while (PyDict_Next(pyitem, &pos, &key, &value)) 
						{
							wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(PyDict_GetItemString(value, "name"), NULL);
							char* name = wchar2char(PyUnicode_AsWideCharStringRet0);
							PyMem_Free(PyUnicode_AsWideCharStringRet0);
							
							KBEngine::DBID dbid = 0;
							dbid = PyLong_AsUnsignedLongLong(key);
 							if (PyErr_Occurred())																	
 							{																						
								dbid = PyLong_AsUnsignedLong(key);														
							}	
 							if (PyErr_Occurred())																	
 							{																						
								PyErr_PrintEx(0);																	
							}	

							Ogre::String str = Ogre::String(name) + "_" + KBEngine::StringConv::val2str(dbid);
							g_avatars.push_back(str);
							mTrayMgr->createButton(OgreBites::TL_CENTER, str, str, 300);

							free(name);
						}																					
					}
				}
			}
			break;
		default:
			break;
	};
}