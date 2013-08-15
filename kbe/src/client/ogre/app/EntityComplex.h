#ifndef __ENTITY_CLIENTCOMPLEX_OGRE_H__
#define __ENTITY_CLIENTCOMPLEX_OGRE_H__

#include "Entity.h"

using namespace Ogre;

class EntityComplex: public KBEntity
{
private:

	// all the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together
	enum AnimID
	{
		ANIM_IDLE_BASE,
		ANIM_IDLE_TOP,
		ANIM_RUN_BASE,
		ANIM_RUN_TOP,
		ANIM_HANDS_CLOSED,
		ANIM_HANDS_RELAXED,
		ANIM_DRAW_SWORDS,
		ANIM_SLICE_VERTICAL,
		ANIM_SLICE_HORIZONTAL,
		ANIM_DANCE,
		ANIM_JUMP_START,
		ANIM_JUMP_LOOP,
		ANIM_JUMP_END,
		ANIM_NONE
	};

public:
	
	EntityComplex(SpaceWorld* pSpace, KBEngine::ENTITY_ID eid);
	~EntityComplex();
	
	virtual Ogre::Real calcDistanceAboveTerrain(){ 
		return 1.5; 
	}

	virtual void addTime(Real deltaTime);

	virtual void onStateChanged()
	{
		if(inWorld_)
		{
			setTopAnimation(ANIM_DRAW_SWORDS, true);
			mTimer = 0;
		}
	}

	void attack(KBEntity* receiver, uint32 skillID, uint32 damageType, uint32 damage);

	void injectKeyDown(const OIS::KeyEvent& evt);
	void injectKeyUp(const OIS::KeyEvent& evt);

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS) || (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID)
	void injectMouseMove(const OIS::MultiTouchEvent& evt);
	void injectMouseDown(const OIS::MultiTouchEvent& evt);
#else
	void injectMouseMove(const OIS::MouseEvent& evt);
	void injectMouseDown(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
#endif

	void setupBody(SceneManager* sceneMgr);

	void setupAnimations();

	void setupCamera(Camera* cam);

	void updateBody(Real deltaTime);
	void updateAnimations(Real deltaTime);
	void fadeAnimations(Real deltaTime);
	void updateCamera(Real deltaTime);
	void updateCameraGoal(Real deltaYaw, Real deltaPitch, Real deltaZoom);
	void setBaseAnimation(AnimID id, bool reset = false);
	void setTopAnimation(AnimID id, bool reset = false);

	virtual void onEnterWorld()
	{
		if(mState == 3)
		{
			if(mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP)
			{
				setTopAnimation(ANIM_DRAW_SWORDS, true);
				mTimer = 0;
			}
		}
	}

	virtual void onLeaveWorld()
	{
	}

private:
	SceneNode* mCameraPivot;
	SceneNode* mCameraGoal;
	SceneNode* mCameraNode;

	Real mPivotPitch;

	Entity* mSword1;
	Entity* mSword2;

	RibbonTrail* mSwordTrail;

	AnimationState* mAnims[NUM_ANIMS];		 // master animation list
	AnimID mBaseAnimID;						 // current base (full- or lower-body) animation
	AnimID mTopAnimID;						 // current top (upper-body) animation

	bool mFadingIn[NUM_ANIMS];				 // which animations are fading in
	bool mFadingOut[NUM_ANIMS];				 // which animations are fading out

	bool mSwordsDrawn;

	Real mTimer;							 // general timer to see how long animations have been playing
	WeaponTrail* pWeaponTrailLeft_, *pWeaponTrailRight_;
};

#endif
