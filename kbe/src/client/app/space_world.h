
#ifndef __SPACEWORLD_CLIENT_h_
#define __SPACEWORLD_CLIENT_h_

#include "space.h"
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include <OgreImage.h>

class DotSceneLoader;

namespace Forests
{
    class PagedGeometry;
    class GrassLoader;
    class GrassLayer;
}

class SpaceWorld : public Space
{
public:
    SpaceWorld(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr);
    virtual ~SpaceWorld(void);

    virtual void setupResources();
    virtual void createScene(void);
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
private:
    std::vector<Ogre::String> mCamNames;
    void switchCamera(int idx);
    Ogre::TerrainGroup* mTerrainGroup;
    DotSceneLoader* mLoader;
    bool mTerrainImported;
    Ogre::String mSceneFile;
    Ogre::String mHelpInfo;
    bool mFly;
    Ogre::Real mFallVelocity;

    Forests::PagedGeometry* mPGHandle;                         /** Handle to Forests::PagedGeometry object */
    Forests::GrassLoader* mGrassLoaderHandle;                /** Handle to Forests::GrassLoader object */
    Forests::GrassLayer* mPGLayers[4];
    float* mPGLayerData[4];
    Ogre::Image mPGDensityMap;
    Ogre::Rect mPGDirtyRect;
};

#endif // #ifndef __SPACEWORLD_CLIENT_h_
