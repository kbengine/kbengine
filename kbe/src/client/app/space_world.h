
#ifndef __SPACEWORLD_CLIENT_h_
#define __SPACEWORLD_CLIENT_h_

#include "space.h"
#include "Entity.h"
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
	typedef std::map<KBEngine::ENTITY_ID, std::tr1::shared_ptr<KBEntity> > ENTITIES;

    SpaceWorld(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr);
    virtual ~SpaceWorld(void);

    virtual void setupResources();
    virtual void createScene(void);
    virtual bool keyPressed( const OIS::KeyEvent &arg );
	virtual bool keyReleased(const OIS::KeyEvent &arg);
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual void kbengine_onEvent(const KBEngine::EventData* lpEventData);
	
	void buttonHit(OgreBites::Button* button);

	DotSceneLoader* getDotSceneLoader(){ return mLoader; }

	float getPositionHeight(const Ogre::Vector3& pos);
	void moveDecalTo(Ogre::ManualObject* pDecal, const Ogre::Vector3& position, float radius);
	Ogre::ManualObject* createDecal(Ogre::String decalMaterialName);
private:
    Ogre::TerrainGroup* mTerrainGroup;
    DotSceneLoader* mLoader;
    bool mTerrainImported;
    Ogre::String mSceneFile;
    Ogre::String mHelpInfo;
    bool mFly;

    Forests::PagedGeometry* mPGHandle;                         /** Handle to Forests::PagedGeometry object */
    Forests::GrassLoader* mGrassLoaderHandle;                /** Handle to Forests::GrassLoader object */
    Forests::GrassLayer* mPGLayers[4];
    float* mPGLayerData[4];
    Ogre::Image mPGDensityMap;
    Ogre::Rect mPGDirtyRect;

	KBEntity* mPlayerPtr;
	KBEntity* mTargetPtr;

	ENTITIES mEntities;

	bool serverClosed_, showCloseButton_, createdReliveButton_;

	Ogre::ManualObject*		pDecalObj_, *pSelDecalObj_;

	Ogre::Vector3 selPos_;
	bool showSelPosDecal_;
};

#endif // #ifndef __SPACEWORLD_CLIENT_h_
