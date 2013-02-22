
#ifndef __OgreApplication_h_
#define __OgreApplication_h_

#include "BaseApplication.h"
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

class OgreApplication : public BaseApplication
{
public:
    OgreApplication(void);
    virtual ~OgreApplication(void);

	virtual void go(void);
protected:
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

#endif // #ifndef __OgreApplication_h_
