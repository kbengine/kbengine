#include "Entity.h"
#include "space_world.h"
#include "DotSceneLoader.h"
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include "../kbengine_dll/kbengine_dll.h"

//-------------------------------------------------------------------------------------
void KBEntity::addTime(Real deltaTime)
{
	if(mBodyNode)
	{
		updateBody(deltaTime);
		updateAnimations(deltaTime);
	}

	Ogre::Vector3 currpos;

	if(mCamera)
	{
		currpos = getPosition();
		updateCamera(deltaTime);
	}
	else
	{
		currpos = getLastPosition();
		if(kbe_playerID() != mID)
		{
			Ogre::Vector3 movement = destPos_ - currpos;
			float speed = mMoveSpeed * deltaTime;

			movement.y = 0.f;

			Real mlen = movement.length();

			if(mlen < speed || (mBaseAnimID != ANIM_RUN_BASE && mlen <= 1.5f))
			{
				if (mBaseAnimID == ANIM_RUN_BASE)
				{
					float y = currpos.y;
					currpos = destPos_;
					currpos.y = y;

					setBaseAnimation(ANIM_IDLE_BASE);
					if (mTopAnimID == ANIM_RUN_TOP) 
						setTopAnimation(ANIM_IDLE_TOP);
				}
			}
			else
			{
				movement.normalise();

				// ÒÆ¶¯Î»ÖÃ
				movement *= speed;
				currpos += movement;
				
				if (mBaseAnimID == ANIM_IDLE_BASE) 
				{
					setBaseAnimation(ANIM_RUN_BASE, true);
					if (mTopAnimID == ANIM_IDLE_TOP) 
					{
						setTopAnimation(ANIM_RUN_TOP, true);
					}
				}
			}
		}
	}

	if(!isJump())
	{
		Ogre::Ray ray;
		ray.setOrigin(Ogre::Vector3(currpos.x, 10000, currpos.z));
		ray.setDirection(Ogre::Vector3::NEGATIVE_UNIT_Y);

		Ogre::TerrainGroup::RayResult rayResult = mSpacePtr->getDotSceneLoader()->getTerrainGroup()->rayIntersects(ray);
		Ogre::Real distanceAboveTerrain = CHAR_HEIGHT;
		Ogre::Real fallSpeed = 200;
		Ogre::Real newy = currpos.y;

		if (rayResult.hit)
		{
			if (currpos.y > rayResult.position.y + distanceAboveTerrain)
			{
				mFallVelocity += deltaTime * 10;
				mFallVelocity = std::min(mFallVelocity, fallSpeed);
				newy = currpos.y - mFallVelocity * deltaTime;
			}

			newy = std::max(rayResult.position.y + distanceAboveTerrain, newy);
			currpos.y = newy;
		}
	}

	setPosition(currpos.x, currpos.y, currpos.z);

	if(!mCamera)
		setDirection(destDir_.z, destDir_.y, destDir_.x);
}

//-------------------------------------------------------------------------------------
bool KBEntity::_checkJumpEnd()
{
	Ogre::Vector3 camPos = getPosition();
	Ogre::Ray ray;
	ray.setOrigin(Ogre::Vector3(camPos.x, 10000, camPos.z));
	ray.setDirection(Ogre::Vector3::NEGATIVE_UNIT_Y);

	Ogre::TerrainGroup::RayResult rayResult = mSpacePtr->getDotSceneLoader()->getTerrainGroup()->rayIntersects(ray);

	if (rayResult.hit)
	{
		if (camPos.y <= rayResult.position.y + CHAR_HEIGHT)
		{
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------