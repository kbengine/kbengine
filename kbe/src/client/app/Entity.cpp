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

				// 移动位置
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
		Ogre::Real distanceAboveTerrain = CHAR_HEIGHT * mScale;
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
		if (camPos.y <= rayResult.position.y + (CHAR_HEIGHT * mScale))
		{
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
void KBEntity::setupBody(SceneManager* sceneMgr)
{
	mSceneMgr = sceneMgr;
	// create main model
	mBodyNode = sceneMgr->getRootSceneNode()->createChildSceneNode(Vector3::UNIT_Y * CHAR_HEIGHT * mScale);

	Ogre::String sKey = Ogre::StringConverter::toString(mID);
	mBodyEnt = sceneMgr->createEntity(sKey + "Body", "Sinbad.mesh");
	mBodyNode->attachObject(mBodyEnt);

	// create swords and attach to sheath
	LogManager::getSingleton().logMessage("Creating swords");
	mSword1 = sceneMgr->createEntity(sKey + "Sword1", "Sword.mesh");
	mSword2 = sceneMgr->createEntity(sKey + "Sword2", "Sword.mesh");
	mBodyEnt->attachObjectToBone("Sheath.L", mSword1);
	mBodyEnt->attachObjectToBone("Sheath.R", mSword2);

	LogManager::getSingleton().logMessage("Creating the chains");
	// create a couple of ribbon trails for the swords, just for fun
	NameValuePairList params;
	params["numberOfChains"] = "2";
	params["maxElements"] = "80";
	mSwordTrail = (RibbonTrail*)sceneMgr->createMovableObject("RibbonTrail", &params);
	mSwordTrail->setMaterialName("Examples/LightRibbonTrail");
	mSwordTrail->setTrailLength(20);
	mSwordTrail->setVisible(false);
	sceneMgr->getRootSceneNode()->attachObject(mSwordTrail);


	for (int i = 0; i < 2; i++)
	{
		mSwordTrail->setInitialColour(i, 1, 0.8, 0);
		mSwordTrail->setColourChange(i, 0.75, 1.25, 1.25, 1.25);
		mSwordTrail->setWidthChange(i, 1);
		mSwordTrail->setInitialWidth(i, 0.3);
	}

	mKeyDirection = Vector3::ZERO;
	mVerticalVelocity = 0;

	// 绑定名称
	Ogre::SceneNode *LabelNode = mBodyNode->createChildSceneNode(Ogre::StringConverter::toString(mID));
	Ogre::MovableText *Label = new Ogre::MovableText(Ogre::StringConverter::toString(mID), mName, 
		"Yahei", 1.0f, Ogre::ColourValue::Black);

	Label->setTextAlignment(Ogre::MovableText::H_CENTER, Ogre::MovableText::V_ABOVE); 
	Label->showOnTop(true);
	Label->setColor(Ogre::ColourValue::Blue);

	Ogre::Vector3 pos = mBodyNode->getPosition();
	LabelNode->attachObject(Label);
	pos.y = pos.y + mBodyEnt->getBoundingBox().getSize().y  * mScale;
	LabelNode->setPosition(pos);

	//scale(mScale);
}


//-------------------------------------------------------------------------------------