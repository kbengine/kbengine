#ifndef __Sinbad_H__
#define __Sinbad_H__

#include "Ogre.h"
#include "OIS.h"
#include "OgreMovableText.h"
#include "cstdkbe/cstdkbe.hpp"

using namespace Ogre;

class SpaceWorld;

#define NUM_ANIMS 13		  // number of animations the character has
#define CHAR_HEIGHT 5         // height of character's center of mass above ground
#define CAM_HEIGHT 2          // height of camera above character's center of mass
#define RUN_SPEED 17          // character running speed in units per second
#define TURN_SPEED 500.0f     // character turning in degrees per second
#define ANIM_FADE_SPEED 7.5f  // animation crossfade speed in % of full weight per second
#define JUMP_ACCEL 30.0f      // character jump acceleration in upward units per squared second
#define GRAVITY 90.0f         // gravity in downward units per squared second

class KBEntity
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
	
	KBEntity(SpaceWorld* pSpace, KBEngine::ENTITY_ID eid);
	~KBEntity();

	KBEngine::ENTITY_ID id()const{ return mID; }

	Ogre::Real calcDistanceAboveTerrain(){ return CHAR_HEIGHT * mScale; }
	void visable(bool v){

		if(mBodyNode)
			mBodyNode->setVisible(v);
	}

	void addTime(Real deltaTime);
	
	void scale(float x, float y, float z)
	{
		mScale = (x + y + z) / 3.0f;

		if(mBodyNode)
			mBodyNode->setScale(x, y, z);
	}

	void scale(float v)
	{
		mScale = v;
		
		if(mBodyNode)
		{
			mBodyNode->setScale(v, v, v);
		}
	}

	void setState(int state)
	{
		mState = state;

		if(inWorld_)
		{
			setTopAnimation(ANIM_DRAW_SWORDS, true);
			mTimer = 0;
		}
	}
	
	void setHPMAX(int v)
	{
		hp_max_ = v;
	}
	
	void setMPMAX(int v)
	{
		mp_max_ = v;
	}

	void attack(KBEntity* receiver, uint32 skillID, uint32 damageType, uint32 damage);
	void recvDamage(KBEntity* attacker, uint32 skillID, uint32 damageType, uint32 damage);

	int getState()
	{
		return mState;
	}
	
	void setHighlighted( bool highlight );

	void setDestDirection(float yaw, float pitch, float roll)
	{
		destDir_.x = roll;
		destDir_.y = pitch;
		destDir_.z = yaw;
	}

	void setDestPosition(float x, float y, float z)
	{
		destPos_.x = x;
		destPos_.y = y;
		destPos_.z = z;
	}

	void setDirection(float yaw, float pitch, float roll)
	{
		//Ogre::Vector3 dir(-yaw, roll, pitch);
		//mBodyNode->setFixedYawAxis(true); 
		//mBodyNode->setDirection(dir, Ogre::Node::TS_WORLD);
	
		if(mBodyNode) mBodyNode->resetOrientation();
		if(mBodyNode) mBodyNode->yaw(Ogre::Radian(yaw));
		currDir_.z = yaw;

		if(mBodyNode) mBodyNode->pitch(Ogre::Radian(pitch));
		currDir_.y = pitch;


		if(mBodyNode) mBodyNode->roll(Ogre::Radian(roll));
		currDir_.x = roll;
	}

	void setPosition(float x, float y, float z)
	{
		lastPos_.x = x;
		lastPos_.y = y;
		lastPos_.z = z;

		if(mBodyNode)
			mBodyNode->setPosition(x, y, z);
	}

	const Ogre::Vector3& getLastPosition()
	{
		return lastPos_;
	}
	
	const Ogre::Vector3& getPosition()
	{
		return mBodyNode->getPosition();
	}
	
	const Ogre::Vector3& getDirection()
	{
		return mGoalDirection;
	}
	
	bool isJump()const { return mIsJump; }

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

	void setMoveSpeed(float speed){ 
		mMoveSpeed = speed; 
	}

	void setName(const Ogre::DisplayString& name){ mName = name; }

	void inWorld(bool v)
	{ 
		inWorld_ = v; 

		if(inWorld_)
			onEnterWorld();
		else
			onLeaveWorld();
	}

	void onEnterWorld()
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

	void onLeaveWorld()
	{
	}

private:
	bool _checkJumpEnd();
private:
	Camera* mCamera;
	SceneNode* mBodyNode;

	SceneNode* mCameraPivot;
	SceneNode* mCameraGoal;
	SceneNode* mCameraNode;

	Real mPivotPitch;

	Entity* mBodyEnt;
	Entity* mSword1;
	Entity* mSword2;

	RibbonTrail* mSwordTrail;

	AnimationState* mAnims[NUM_ANIMS];		 // master animation list
	AnimID mBaseAnimID;						 // current base (full- or lower-body) animation
	AnimID mTopAnimID;						 // current top (upper-body) animation

	bool mFadingIn[NUM_ANIMS];				 // which animations are fading in
	bool mFadingOut[NUM_ANIMS];				 // which animations are fading out

	bool mSwordsDrawn;

	Vector3 mKeyDirection;					 // player's local intended direction based on WASD keys
	Vector3 mGoalDirection;					 // actual intended direction in world-space
	Real mVerticalVelocity;					 // for jumping

	Ogre::Real mFallVelocity;				// 掉落的速度

	Real mTimer;							 // general timer to see how long animations have been playing

	float mMoveSpeed;						// 移动速度
	float mScale;							// 模型缩放比

	Ogre::DisplayString mName;				// 名称
	KBEngine::ENTITY_ID mID;				// entityID

	bool mIsJump;
	
	SpaceWorld*	mSpacePtr;

	SceneManager* mSceneMgr;

	Ogre::Vector3 destPos_, lastPos_, destDir_, currDir_;
	
	int mState;

	bool inWorld_;
	
	Ogre::SceneNode *pNameLabelNode_;
	Ogre::MovableText *pNameLabel_;

	Ogre::SceneNode *pDamageLabelNode_;
	Ogre::MovableText *pDamageLabel_;
	Real damageShowTime_;

	Ogre::SceneNode *pHudNode_;
	BillboardSet* pHealthHUD_;

	int hp_, hp_max_, mp_, mp_max_;
};

#endif
