#ifndef __ENTITY_CLIENT_OGRE_H__
#define __ENTITY_CLIENT_OGRE_H__

#include "Ogre.h"
#include "OIS.h"
#include "OgreMovableText.h"
#include "cstdkbe/cstdkbe.hpp"

using namespace Ogre;

class SpaceWorld;
class WeaponTrail;

#define NUM_ANIMS 13		  // number of animations the character has
#define CAM_HEIGHT 2          // height of camera above character's center of mass
#define RUN_SPEED 17          // character running speed in units per second
#define TURN_SPEED 500.0f     // character turning in degrees per second
#define ANIM_FADE_SPEED 7.5f  // animation crossfade speed in % of full weight per second
#define JUMP_ACCEL 30.0f      // character jump acceleration in upward units per squared second
#define GRAVITY 90.0f         // gravity in downward units per squared second

class KBEntity
{
public:
	
	KBEntity(SpaceWorld* pSpace, KBEngine::ENTITY_ID eid);
	virtual ~KBEntity();

	KBEngine::ENTITY_ID id()const{ return mID; }

	virtual Ogre::Real calcDistanceAboveTerrain(){ return char_height_; }

	void visable(bool v){

		if(mBodyNode)
			mBodyNode->setVisible(v);
	}

	virtual void addTime(Real deltaTime);
	
	void showBoundingBoxes(bool v){
		mBodyNode->showBoundingBox(v);
	}

	virtual Ogre::Real getHeight(){ return char_height_; }

	virtual void scale(float x, float y, float z)
	{
		mScale = (x + y + z) / 3.0f;
		onScale();
	}

	virtual void onScale()
	{
		if(mBodyNode)
		{
			mBodyNode->setScale(mScale, mScale, mScale);
			char_height_ = mBodyEnt->getWorldBoundingBox(true).getSize().y;
		}
	}

	virtual void scale(float v)
	{
		mScale = v;
		onScale();
	}

	void stickto(Real deltaTime);

	virtual void setState(int state)
	{
		mState = state;
		onStateChanged();
	}
	
	virtual void onStateChanged()
	{
		
	}
	
	void setHPMAX(int v)
	{
		hp_max_ = v;
	}
	
	void setMPMAX(int v)
	{
		mp_max_ = v;
	}

	virtual void attack(KBEntity* receiver, uint32 skillID, uint32 damageType, uint32 damage);
	virtual void recvDamage(KBEntity* attacker, uint32 skillID, uint32 damageType, uint32 damage);

	int getState()
	{
		return mState;
	}
	
	void setHighlighted( bool highlight );

	virtual void setDestDirection(float yaw, float pitch, float roll)
	{
		destDir_.x = roll;
		destDir_.y = pitch;
		destDir_.z = yaw;
	}

	virtual void setDestPosition(float x, float y, float z)
	{
		destPos_.x = x;
		destPos_.y = y;
		destPos_.z = z;
	}

	virtual void setDirection(float yaw, float pitch, float roll)
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

	virtual void setPosition(float x, float y, float z)
	{
		lastPos_.x = x;
		lastPos_.y = y;
		lastPos_.z = z;

		if(mBodyNode)
		{
			Ogre::Vector3 pos(x, y, z);

			mBodyNode->setPosition(pos);

			pos.y += char_height_ - calcDistanceAboveTerrain();

			pNameLabelNode_->setPosition(pos);

			pos.y += 1.f;
			pHudNode_->setPosition(pos);

			pos.y += 1.f;
			pDamageLabelNode_->setPosition(pos);
		}
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

	virtual void injectKeyDown(const OIS::KeyEvent& evt);
	virtual void injectKeyUp(const OIS::KeyEvent& evt);

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS) || (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID)
	virtual void injectMouseMove(const OIS::MultiTouchEvent& evt);
	virtual void injectMouseDown(const OIS::MultiTouchEvent& evt);
#else
	virtual void injectMouseMove(const OIS::MouseEvent& evt);
	virtual void injectMouseDown(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
#endif

	virtual void setup(SceneManager* sceneMgr);
	
	virtual void setupBody(SceneManager* sceneMgr);

	virtual void setupAnimations();

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

	virtual void onEnterWorld()
	{
	}

	virtual void onLeaveWorld()
	{
	}

	void setModelID(Ogre::uint32 modelID){ modelID_ = modelID; }
protected:
	virtual bool _checkJumpEnd();
protected:
	Camera* mCamera;

	SceneNode* mBodyNode;

	Entity* mBodyEnt;

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

	float char_height_; // height of character's center of mass above ground

	Ogre::String modelName_;

	Ogre::uint32 modelID_;
};

#endif
