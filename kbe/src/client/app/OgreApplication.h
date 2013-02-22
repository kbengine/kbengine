
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

class OgreApplication : public BaseApplication, public Ogre::Singleton<OgreApplication>
{
public:
    OgreApplication(void);
    virtual ~OgreApplication(void);

	virtual void go(void);

	void setCurrCameraMan(OgreBites::SdkCameraMan* pCameraMan){ mCameraMan = pCameraMan; }

	virtual void buttonHit(OgreBites::Button* button);
protected:
	virtual bool setup();
    virtual void setupResources();
	virtual void createScene(void){}
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
private:
};

#endif // #ifndef __OgreApplication_h_
