
#ifndef __SPACE_CLIENT_h_
#define __SPACE_CLIENT_h_

#include "BaseApplication.h"
#include "client_lib/event.hpp"

class Space
{
public:
	enum QueryFlags
	{
		ENTITY_MASK = 1 << 0,
		SCENE_MASK = 1 << 2
	};

    Space(Ogre::Root *pOgreRoot, Ogre::RenderWindow* pRenderWin, 
		OIS::InputManager* pInputMgr, OgreBites::SdkTrayManager* pTrayMgr);

    virtual ~Space(void);

	virtual bool setup();
    virtual void setupResources() = 0;
    virtual void createScene(void) = 0;
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) = 0;

	virtual bool keyPressed(const OIS::KeyEvent &arg){ return true; }
	virtual bool keyReleased(const OIS::KeyEvent &arg){ return true; }
    virtual bool mouseMoved( const OIS::MouseEvent &arg ){ return true; }
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ){ return true; }
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ){ return true; }
	virtual void buttonHit(OgreBites::Button* button){}

	virtual void kbengine_onEvent(const KBEngine::EventData* lpEventData){};

	bool pickEntity(Ogre::RaySceneQuery* mRaySceneQuery, Ogre::Ray &ray, Ogre::Entity **result, Ogre::uint32 querymask, Ogre::uint32 mVisibilityMask, Ogre::Vector3 &hitpoint, const Ogre::String& excludeobject, Ogre::Real max_distance);

	void getMeshData(const Ogre::MeshPtr mesh, size_t &vertex_count, size_t &index_count,
                               const Ogre::Vector3 &position, const Ogre::Quaternion &orient, const Ogre::Vector3 &scale);
protected:
    Ogre::Root *mRoot;
    Ogre::Camera* mActiveCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;

	OIS::InputManager* mInputManager;
	OgreBites::SdkTrayManager*    mTrayMgr;
	OgreBites::SdkCameraMan* mCameraMan;

	Ogre::RaySceneQuery* mRaySceneQuery;
	unsigned int   mVertexBufferSize;
	unsigned int   mIndexBufferSize;
	Ogre::Vector3 *mVertexBuffer;
	unsigned long *mIndexBuffer;
};

#endif // #ifndef __SPACE_CLIENT_h_
