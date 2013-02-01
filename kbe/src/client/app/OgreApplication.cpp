#include "OgreApplication.h"
#include "DotSceneLoader.h"
#include "PagedGeometry.h"
#include "GrassLoader.h"
#include "BatchPage.h"
#include "ImpostorPage.h"
#include "TreeLoader3D.h"

#include "../kbengine_dll/kbengine_dll.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
HINSTANCE g_hKBEngineDll = NULL;
#endif

//-------------------------------------------------------------------------------------
OgreApplication::OgreApplication(void)
:    mLoader(0),
    mTerrainImported(true),
    mSceneFile(Ogre::StringUtil::BLANK),
    mHelpInfo(Ogre::StringUtil::BLANK),
    mFly(false),
    mFallVelocity(0)
{
    mCamNames.clear();

    mHelpInfo = Ogre::String("Use [W][A][S][D] keys for movement.\nKeys [1]-[9] to switch between cameras.\n[0] toggles SceneNode debug visuals.\n\nPress [C] to toggle clamp to terrain (gravity).\n\n[G] toggles the detail panel.\n[R] cycles polygonModes (Solid/Wireframe/Points).\n[T] cycles various filtering.\n\n\nPress [ESC] to quit.");
}

//-------------------------------------------------------------------------------------
OgreApplication::~OgreApplication(void)
{
	delete mLoader;
}

//-------------------------------------------------------------------------------------
void OgreApplication::setupResources(void)
{
    BaseApplication::setupResources();

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
        "" + projectDir, "FileSystem", "General");

    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/SampleScene2", "FileSystem", "General");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Materials", "FileSystem", "General");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Models", "FileSystem", "General");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        "" + projectDir + "/Terrain", "FileSystem", "General");
}

//-------------------------------------------------------------------------------------
void OgreApplication::createScene(void)
{
    mLoader = new DotSceneLoader();
    mLoader->parseDotScene(mSceneFile, "General", mSceneMgr);

    // Get rid of the initial camera
    mSceneMgr->destroyCamera(mCamera);

    // Loop through all cameras and grab their name and set their debug representation
    Ogre::SceneManager::CameraIterator cameras = mSceneMgr->getCameraIterator();
    while (cameras.hasMoreElements())
    {
        Ogre::Camera* camera = cameras.getNext();
        mCamNames.push_back(camera->getName());
        Ogre::Entity* debugEnt = mSceneMgr->createEntity(camera->getName() + Ogre::String("_debug"), "scbCamera.mesh");

        try{
            Ogre::SceneNode* sNode = mSceneMgr->getSceneNode(camera->getName());
            sNode->attachObject(debugEnt);
            sNode->scale(0.5, 0.5, 0.5);
        }catch (...){
            Ogre::SceneNode* pNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(camera->getName());
            pNode->setPosition(camera->getPosition());
            pNode->setOrientation(camera->getOrientation());

            pNode->attachObject(debugEnt);
            pNode->scale(0.5, 0.5, 0.5);
        }
    }
    // Grab the first available camera, for now
    Ogre::String cameraName = mCamNames[0];
    try
    {
        mActiveCamera = mSceneMgr->getCamera(cameraName);
        mWindow->getViewport(0)->setCamera(mActiveCamera);
        mCameraMan->setCamera(mActiveCamera);
        mSceneMgr->getEntity(mActiveCamera->getName() + Ogre::String("_debug"))->setVisible(false);

        for(unsigned int ij = 0;ij < mLoader->mPGHandles.size();ij++)
        {
            mLoader->mPGHandles[ij]->setCamera(mActiveCamera);
        }
        
    }
    catch (Ogre::Exception& e)
    {
        Ogre::LogManager::getSingleton().logMessage("OgreApplication::createScene : setting the active camera to (\"" +
            cameraName + ") failed: " + e.getFullDescription());
    }
}

//-------------------------------------------------------------------------------------
bool OgreApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    bool ret = BaseApplication::frameRenderingQueued(evt);

    if (!mFly)
    {
        // clamp to terrain
        Ogre::Vector3 camPos = mActiveCamera->getPosition();
        Ogre::Ray ray;
        ray.setOrigin(Ogre::Vector3(camPos.x, 10000, camPos.z));
        ray.setDirection(Ogre::Vector3::NEGATIVE_UNIT_Y);

        Ogre::TerrainGroup::RayResult rayResult = mLoader->getTerrainGroup()->rayIntersects(ray);
        Ogre::Real distanceAboveTerrain = 1.4f;
        Ogre::Real fallSpeed = 200;
        Ogre::Real newy = camPos.y;
        if (rayResult.hit)
        {
            if (camPos.y > rayResult.position.y + distanceAboveTerrain)
            {
                mFallVelocity += evt.timeSinceLastFrame * 10;
                mFallVelocity = std::min(mFallVelocity, fallSpeed);
                newy = camPos.y - mFallVelocity * evt.timeSinceLastFrame;

            }
            newy = std::max(rayResult.position.y + distanceAboveTerrain, newy);
            mActiveCamera->setPosition(camPos.x, newy, camPos.z);
        }
    }

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

    return ret;
}

//-------------------------------------------------------------------------------------
void OgreApplication::switchCamera(int idx)
{
    if (idx <= (int)mCamNames.size())
    {
        mSceneMgr->getEntity(mActiveCamera->getName() + Ogre::String("_debug"))->setVisible(true);
        mSceneMgr->getSceneNode(mActiveCamera->getName())->setPosition(mActiveCamera->getPosition());
        mSceneMgr->getSceneNode(mActiveCamera->getName())->setOrientation(mActiveCamera->getOrientation());
        mActiveCamera = mSceneMgr->getCamera(mCamNames[idx-1]);
        mWindow->getViewport(0)->setCamera(mActiveCamera);
        mCameraMan->setCamera(mActiveCamera);
        mSceneMgr->getEntity(mActiveCamera->getName() + Ogre::String("_debug"))->setVisible(false);
    }
}

//-------------------------------------------------------------------------------------
bool OgreApplication::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_H || arg.key == OIS::KC_F1)   // toggle visibility of help dialog
    {
        if (!mTrayMgr->isDialogVisible()) mTrayMgr->showOkDialog("Help", mHelpInfo);
        else mTrayMgr->closeDialog();
    }
    if (arg.key == OIS::KC_0)   // toggle scenenode debug renderables
    {
        mSceneMgr->setDisplaySceneNodes(!mSceneMgr->getDisplaySceneNodes());
    }
    if (arg.key == OIS::KC_1)   // switch to camera 1
    {
        switchCamera(1);
    }
    if (arg.key == OIS::KC_2)   // switch to camera 2
    {
        switchCamera(2);
    }
    if (arg.key == OIS::KC_3)   // switch to camera 3
    {
        switchCamera(3);
    }
    if (arg.key == OIS::KC_4)   // switch to camera 4
    {
        switchCamera(4);
    }
    if (arg.key == OIS::KC_5)   // switch to camera 5
    {
        switchCamera(5);
    }
    if (arg.key == OIS::KC_6)   // switch to camera 6
    {
        switchCamera(6);
    }
    if (arg.key == OIS::KC_7)   // switch to camera 7
    {
        switchCamera(7);
    }
    if (arg.key == OIS::KC_8)   // switch to camera 8
    {
        switchCamera(8);
    }
    if (arg.key == OIS::KC_9)   // switch to camera 9
    {
        switchCamera(9);
    }
    if (arg.key == OIS::KC_C)   // toggle fly/walk
    {
        mFly = !mFly;
    }

    return BaseApplication::keyPressed( arg );
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

		if(!kbengine_init())
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

		kbengine_destroy();

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		FreeLibrary(g_hKBEngineDll);
#endif

        return 0;
    }

#ifdef __cplusplus
}
#endif
