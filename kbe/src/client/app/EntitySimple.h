#ifndef __ENTITY_CLIENTSIMPLE_OGRE_H__
#define __ENTITY_CLIENTSIMPLE_OGRE_H__

#include "Entity.h"

#define SIMPLE_NUM_ANIMS 16

class EntitySimple: public KBEntity
{
private:
	enum AnimID
	{
		ANIM_ATTACK1,
		ANIM_ATTACK2,
		ANIM_ATTACK3,
		ANIM_ATTACK4,
		ANIM_ATTACK5,
		ANIM_IDLE_1,
		ANIM_IDLE_2,
		ANIM_BATTLEIDLE1,
		ANIM_BATTLEIDLE2,
		ANIM_BLOCK,
		ANIM_DIE1,
		ANIM_DIE2,
		ANIM_RUN,
		ANIM_WALK,
		ANIM_JUMP1,
		ANIM_JUMP2
	};
public:
	EntitySimple(SpaceWorld* pSpace, KBEngine::ENTITY_ID eid);
	~EntitySimple();

	virtual Ogre::Real calcDistanceAboveTerrain()
	{ 
		if(modelID_ == 1001)
			return 0.f; 

		return 1.8f; 
	}

	virtual void scale(float x, float y, float z)
	{
		if(modelID_ == 1001)
			mScale = 2.0f;
		else
			mScale = 0.7f;

		onScale();
	}

	virtual void scale(float v)
	{
		if(modelID_ == 1001)
			v = 2.0f;
		else
			v = 0.7f;

		mScale = v;
		onScale();
	}

	virtual void addTime(Real deltaTime);

	virtual void onStateChanged()
	{
		if(inWorld_)
		{
			if(mState == 1)
				playAnimation("Die");
			else
				playAnimation("Idle");
		}
	}

	virtual void onEnterWorld()
	{
		onStateChanged();
	}

	void playAnimation(Ogre::String name);

	void attack(KBEntity* receiver, uint32 skillID, uint32 damageType, uint32 damage);
	virtual void recvDamage(KBEntity* attacker, uint32 skillID, uint32 damageType, uint32 damage);

	void setupBody(SceneManager* sceneMgr);
	void setupAnimations();

private:
	AnimationState* mAnims[SIMPLE_NUM_ANIMS];		 // master animation list
	AnimationState* mLastAnims;
	Ogre::String mLastAnimName;

	Ogre::Real blocktime_;
};

#endif
