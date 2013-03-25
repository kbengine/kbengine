#include "space_world.h"
#include "DotSceneLoader.h"
#include "PagedGeometry.h"
#include "GrassLoader.h"
#include "BatchPage.h"
#include "ImpostorPage.h"
#include "TreeLoader3D.h"
#include "OgreApplication.h"

//-------------------------------------------------------------------------------------
SpaceWorld::SpaceWorld(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr)
:   Space(pOgreRoot, pRenderWin, pInputMgr, pTrayMgr),
	mLoader(0),
    mTerrainImported(true),
    mSceneFile(Ogre::StringUtil::BLANK),
    mHelpInfo(Ogre::StringUtil::BLANK),
    mFly(false),
	mPlayerPtr(0)
{
    mHelpInfo = Ogre::String("Use [W][A][S][D] keys for movement.\nKeys [1]-[9] to switch between cameras.\n[0] toggles SceneNode debug visuals.\n\nPress [C] to toggle clamp to terrain (gravity).\n\n[G] toggles the detail panel.\n[R] cycles polygonModes (Solid/Wireframe/Points).\n[T] cycles various filtering.\n\n\nPress [ESC] to quit.");
}

//-------------------------------------------------------------------------------------
SpaceWorld::~SpaceWorld(void)
{
	mSceneMgr->destroyCamera("mainCamera");
	mActiveCamera = NULL;
	delete mLoader;
	SAFE_RELEASE(mPlayerPtr);
}

//-------------------------------------------------------------------------------------
void SpaceWorld::setupResources(void)
{
    rapidxml::xml_document<> XMLDoc;    // character type defaults to char
    rapidxml::xml_node<>* XMLRoot;

    std::ifstream fp;
    fp.open("SampleAppConfig.xml", std::ios::in | std::ios::binary);
    Ogre::DataStreamPtr stream(OGRE_NEW Ogre::FileStreamDataStream("SampleAppConfig.xml", &fp, false));
    char* sampleAppConfig = strdup(stream->getAsString().c_str());
    XMLDoc.parse<0>(sampleAppConfig);
    XMLRoot = XMLDoc.first_node("SampleApp");

    Ogre::String projectDir = Ogre::String(XMLRoot->first_attribute("projectDir")->value());
    mSceneFile = Ogre::String(XMLRoot->first_attribute("scene")->value());

    // add sample project directory to the resource paths
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir, "FileSystem", "Space");

    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Materials", "FileSystem", "Space");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Models", "FileSystem", "Space");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Terrain", "FileSystem", "Space");

	mTrayMgr->showLoadingBar(1, 0);
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Space");
	mTrayMgr->hideLoadingBar();
}

//-------------------------------------------------------------------------------------
void SpaceWorld::createScene(void)
{
	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

    mLoader = new DotSceneLoader();
    mLoader->parseDotScene(mSceneFile, "Space", mSceneMgr);

	mTrayMgr->showCursor();
	mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	mTrayMgr->showBackdrop();
	mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);

	// set shadow properties
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	mActiveCamera = mSceneMgr->createCamera("mainCamera");
	// Create one viewport, entire window
	Ogre::Viewport* vp = mWindow->addViewport(mActiveCamera);
	vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

	// Alter the camera aspect ratio to match the viewport
	mActiveCamera->setAspectRatio(
		Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

    mWindow->getViewport(0)->setCamera(mActiveCamera);

	mActiveCamera->setNearClipDistance(0.1);
	mActiveCamera->setFarClipDistance(30000);

	mCameraMan = new OgreBites::SdkCameraMan(mActiveCamera);   // create a default camera controller
	mCameraMan->setTopSpeed(7.0f);
    mCameraMan->setCamera(mActiveCamera);
	OgreApplication::getSingleton().setCurrCameraMan(mCameraMan);


	mCameraMan->setStyle(OgreBites::CS_MANUAL);

	mPlayerPtr = new KBEntity(this);
	mPlayerPtr->setupBody(mSceneMgr);
	mPlayerPtr->setupCamera(mActiveCamera);
	mPlayerPtr->setupAnimations();

	mPlayerPtr->setPosition(-97.9299, 191.0, -158.922);
	mPlayerPtr->scale(0.3, 0.3, 0.3);
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if (!mLoader->getTerrainGroup()->isDerivedDataUpdateInProgress())
    {
        if (mTerrainImported)
        {
            mLoader->getTerrainGroup()->saveAllTerrains(true);
            mTerrainImported = false;
        }
    }

    for(unsigned int ij = 0;ij < mLoader->mPGHandles.size();ij++)
    {
        mLoader->mPGHandles[ij]->update();
    }

	if(mPlayerPtr) 
		mPlayerPtr->addTime(evt.timeSinceLastFrame);

    return true;
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::keyPressed( const OIS::KeyEvent &arg )
{
    if (arg.key == OIS::KC_H || arg.key == OIS::KC_F1)   // toggle visibility of help dialog
    {
        if (!mTrayMgr->isDialogVisible()) mTrayMgr->showOkDialog("Help", mHelpInfo);
        else mTrayMgr->closeDialog();
    }
    if (arg.key == OIS::KC_0)   // toggle scenenode debug renderables
    {
        mSceneMgr->setDisplaySceneNodes(!mSceneMgr->getDisplaySceneNodes());
    }
    if (arg.key == OIS::KC_C)   // toggle fly/walk
    {
        mFly = !mFly;
    }

	if(mPlayerPtr) mPlayerPtr->injectKeyDown(arg);
    return true; 
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::keyReleased(const OIS::KeyEvent &arg)
{
	if(mPlayerPtr) mPlayerPtr->injectKeyUp(arg);
    return true;
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::mouseMoved( const OIS::MouseEvent &arg )
{
    if(mPlayerPtr) mPlayerPtr->injectMouseMove(arg);
    return false;
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if(mPlayerPtr) mPlayerPtr->injectMouseDown(arg, id);
    return false;
}

//-------------------------------------------------------------------------------------
bool SpaceWorld::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    return true;
}

//-------------------------------------------------------------------------------------
void SpaceWorld::kbengine_onEvent(const KBEngine::EventData* lpEventData)
{
}
