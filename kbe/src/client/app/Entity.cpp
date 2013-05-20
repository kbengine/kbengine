#include "Entity.h"
#include "WeaponTrail.h"
#include "space_world.h"
#include "DotSceneLoader.h"
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include "../kbengine_dll/kbengine_dll.h"

//-------------------------------------------------------------------------------------
KBEntity::KBEntity(SpaceWorld* pSpace, KBEngine::ENTITY_ID eid):
  mCamera(NULL),
  mBodyNode(NULL),
  mCameraPivot(NULL),
  mCameraGoal(NULL),
  mCameraNode(NULL),
  mPivotPitch(0.0f),
  mBodyEnt(NULL),
  mSword1(NULL),
  mSword2(NULL),
  mSwordTrail(NULL),
  mFallVelocity(0),
  mMoveSpeed(1.0f),
  mScale(0.3f),
  mID(eid),
  mIsJump(false),
  mSpacePtr(pSpace),
  mSceneMgr(NULL),
  destPos_(),
  lastPos_(),
  destDir_(),
  currDir_(),
  mState(0),
  inWorld_(false),
  pNameLabelNode_(NULL),
  pNameLabel_(NULL),
  pDamageLabelNode_(NULL),
  pDamageLabel_(NULL),
  damageShowTime_(0.f),
  pHudNode_(NULL),
  pHealthHUD_(NULL),
  hp_(100),
  hp_max_(100),
  mp_(100),
  mp_max_(100),
  pWeaponTrailLeft_(NULL),
  pWeaponTrailRight_(NULL)
{
	//setupBody(cam->getSceneManager());
	//setupCamera(cam);
	//setupAnimations();
}

//-------------------------------------------------------------------------------------
KBEntity::~KBEntity()
{
	if(pNameLabelNode_ && pNameLabel_)
	{
		pNameLabelNode_->detachObject(pNameLabel_);
		delete pNameLabel_;
	}

	if(pDamageLabel_)
	{
		pDamageLabelNode_->detachObject(pDamageLabel_);
		delete pDamageLabel_;
		pDamageLabel_ = NULL;
	}
	
	SAFE_RELEASE(pWeaponTrailLeft_);
	SAFE_RELEASE(pWeaponTrailRight_);

	if(mBodyEnt)mSceneMgr->destroyEntity(mBodyEnt);
	if(mSword1)mSceneMgr->destroyEntity(mSword1);
	if(mSword2)mSceneMgr->destroyEntity(mSword2);

	if(mSwordTrail)mSceneMgr->destroyMovableObject(mSwordTrail);

	if(mBodyNode)mSceneMgr->destroySceneNode(mBodyNode);
	if(mCameraPivot)mSceneMgr->destroySceneNode(mCameraPivot);
	if(mCameraGoal)mSceneMgr->destroySceneNode(mCameraGoal);
	if(mCameraNode)mSceneMgr->destroySceneNode(mCameraNode);
}

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

			if(mlen < speed || (mBaseAnimID != ANIM_RUN_BASE && mlen <= 1.0f))
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
		Ogre::Real distanceAboveTerrain = calcDistanceAboveTerrain();
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

	if(damageShowTime_ > 0.f)
	{
		damageShowTime_ -= deltaTime;
	}
	else
	{
		if(pDamageLabel_)
		{
			pDamageLabelNode_->detachObject(pDamageLabel_);
			delete pDamageLabel_;
			pDamageLabel_ = NULL;
		}
	}

	if(pWeaponTrailLeft_)
		pWeaponTrailLeft_->onUpdate(deltaTime);

	if(pWeaponTrailRight_)
		pWeaponTrailRight_->onUpdate(deltaTime);
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
	mBodyEnt = sceneMgr->createEntity(sKey + "_Body", "Sinbad.mesh");
	mBodyEnt->setQueryFlags(Space::ENTITY_MASK);
	mBodyNode->attachObject(mBodyEnt);

	// create swords and attach to sheath
	LogManager::getSingleton().logMessage("Creating swords");
	mSword1 = sceneMgr->createEntity(sKey + "_Sword1", "Sword.mesh");
	mSword1->setQueryFlags(Space::ENTITY_MASK);
	mSword2 = sceneMgr->createEntity(sKey + "_Sword2", "Sword.mesh");
	mSword2->setQueryFlags(Space::ENTITY_MASK);
	mBodyEnt->attachObjectToBone("Sheath.L", mSword1);
	mBodyEnt->attachObjectToBone("Sheath.R", mSword2);
	
	//pWeaponTrailLeft_ = new WeaponTrail(sKey + "WeaponTrailL", sceneMgr);
	//pWeaponTrailLeft_->setWeaponEntity(mSword1);
	//pWeaponTrailLeft_->setWidth(2.0f);
	//pWeaponTrailLeft_->setActive(false);

	//pWeaponTrailRight_ = new WeaponTrail(sKey + "WeaponTrailR", sceneMgr);
	//pWeaponTrailRight_->setWeaponEntity(mSword2);
	//pWeaponTrailRight_->setWidth(13.0f);
	//pWeaponTrailRight_->setActive(false);

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
		mSwordTrail->setInitialColour(i, 1, 0.8f, 0);
		mSwordTrail->setColourChange(i, 0.75f, 1.25f, 1.25f, 1.25f);
		mSwordTrail->setWidthChange(i, 1.f);
		mSwordTrail->setInitialWidth(i, 0.3f);
	}

	mKeyDirection = Vector3::ZERO;
	mVerticalVelocity = 0;

	// 绑定名称
	pNameLabelNode_ = mBodyNode->createChildSceneNode(Ogre::StringConverter::toString(mID));
	pNameLabel_ = new Ogre::MovableText(Ogre::StringConverter::toString(mID) + "_name", mName, 
		"Yahei", 1.0f, Ogre::ColourValue::Black);

	pNameLabel_->setTextAlignment(Ogre::MovableText::H_CENTER, Ogre::MovableText::V_ABOVE); 
	pNameLabel_->showOnTop(true);
	pNameLabel_->setColor(Ogre::ColourValue::Blue);

	Ogre::Vector3 pos = mBodyNode->getPosition();
	pNameLabelNode_->attachObject(pNameLabel_);
	pos.y = pos.y + mBodyEnt->getBoundingBox().getSize().y  * mScale + 1.0f;
	pNameLabelNode_->setPosition(pos);

	pDamageLabelNode_ = mBodyNode->createChildSceneNode(Ogre::StringConverter::toString(mID) + "_damage");
	pos.y += (mBodyEnt->getBoundingBox().getSize().y * 0.3f * mScale);
	pDamageLabelNode_->setPosition(pos);

	// 创建头顶血条
	pHealthHUD_ = sceneMgr->createBillboardSet();
	pHealthHUD_->setMaterialName("Examples/Billboard");
	pHealthHUD_->setDefaultWidth(3);
	pHealthHUD_->setDefaultHeight(0.3f);

	pHudNode_ = mBodyNode->createChildSceneNode(Ogre::StringConverter::toString(mID) + "_hud");
	pHudNode_->attachObject(pHealthHUD_);

	Billboard* b = pHealthHUD_->createBillboard(0, mBodyEnt->getBoundingBox().getSize().y * mScale + 2.0f, 0);
	b->setColour(Ogre::ColourValue::Green);
	//b->setDimensions(96, 12);
	b = NULL;

	//scale(mScale);
}

//-------------------------------------------------------------------------------------
void KBEntity::injectKeyDown(const OIS::KeyEvent& evt)
{
	if (evt.key == OIS::KC_Q && (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP))
	{
		// take swords out (or put them back, since it's the same animation but reversed)
		setTopAnimation(ANIM_DRAW_SWORDS, true);
		mTimer = 0;
	}
	else if (evt.key == OIS::KC_E && !mSwordsDrawn)
	{
		if (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP)
		{
			// start dancing
			setBaseAnimation(ANIM_DANCE, true);
			setTopAnimation(ANIM_NONE);
			// disable hand animation because the dance controls hands
			mAnims[ANIM_HANDS_RELAXED]->setEnabled(false);
		}
		else if (mBaseAnimID == ANIM_DANCE)
		{
			// stop dancing
			setBaseAnimation(ANIM_IDLE_BASE);
			setTopAnimation(ANIM_IDLE_TOP);
			// re-enable hand animation
			mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
		}
	}

	// keep track of the player's intended direction
	else if (evt.key == OIS::KC_W) mKeyDirection.z = -1;
	else if (evt.key == OIS::KC_A) mKeyDirection.x = -1;
	else if (evt.key == OIS::KC_S) mKeyDirection.z = 1;
	else if (evt.key == OIS::KC_D) mKeyDirection.x = 1;

	else if (evt.key == OIS::KC_SPACE && (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP))
	{
		// jump if on ground
		setBaseAnimation(ANIM_JUMP_START, true);
		setTopAnimation(ANIM_NONE);
		mTimer = 0;
	}

	if (!mKeyDirection.isZeroLength() && mBaseAnimID == ANIM_IDLE_BASE)
	{
		// start running if not already moving and the player wants to move
		setBaseAnimation(ANIM_RUN_BASE, true);
		if (mTopAnimID == ANIM_IDLE_TOP) setTopAnimation(ANIM_RUN_TOP, true);
	}
}

//-------------------------------------------------------------------------------------
void KBEntity::injectKeyUp(const OIS::KeyEvent& evt)
{
	// keep track of the player's intended direction
	if (evt.key == OIS::KC_W && mKeyDirection.z == -1) mKeyDirection.z = 0;
	else if (evt.key == OIS::KC_A && mKeyDirection.x == -1) mKeyDirection.x = 0;
	else if (evt.key == OIS::KC_S && mKeyDirection.z == 1) mKeyDirection.z = 0;
	else if (evt.key == OIS::KC_D && mKeyDirection.x == 1) mKeyDirection.x = 0;

	if (mKeyDirection.isZeroLength() && mBaseAnimID == ANIM_RUN_BASE)
	{
		// stop running if already moving and the player doesn't want to move
		setBaseAnimation(ANIM_IDLE_BASE);
		if (mTopAnimID == ANIM_RUN_TOP) setTopAnimation(ANIM_IDLE_TOP);
	}
}

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS) || (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID)
//-------------------------------------------------------------------------------------
void KBEntity::injectMouseMove(const OIS::MultiTouchEvent& evt)
{
	// update camera goal based on mouse movement
	updateCameraGoal(-0.05f * evt.state.X.rel, -0.05f * evt.state.Y.rel, -0.1f * evt.state.Z.rel);
}

//-------------------------------------------------------------------------------------
void KBEntity::injectMouseDown(const OIS::MultiTouchEvent& evt)
{
	if (mSwordsDrawn && (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP))
	{
		// if swords are out, and character's not doing something weird, then SLICE!
        setTopAnimation(ANIM_SLICE_VERTICAL, true);
		mTimer = 0;
	}
}
#else
//-------------------------------------------------------------------------------------
void KBEntity::injectMouseMove(const OIS::MouseEvent& evt)
{
	// update camera goal based on mouse movement
	if (evt.state.buttonDown(OIS::MB_Right)) //按下右键后旋转
	{
		updateCameraGoal(-0.1f * evt.state.X.rel, -0.1f * evt.state.Y.rel, -0.1f * evt.state.Z.rel);
	}
}

//-------------------------------------------------------------------------------------
void KBEntity::injectMouseDown(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
	if (mSwordsDrawn && (mTopAnimID == ANIM_IDLE_TOP || mTopAnimID == ANIM_RUN_TOP))
	{
		// if swords are out, and character's not doing something weird, then SLICE!
		if (id == OIS::MB_Left) setTopAnimation(ANIM_SLICE_VERTICAL, true);
		else if (id == OIS::MB_Right) setTopAnimation(ANIM_SLICE_HORIZONTAL, true);
		mTimer = 0;
	}
}
#endif

//-------------------------------------------------------------------------------------
void KBEntity::setupAnimations()
{
	// this is very important due to the nature of the exported animations
	mBodyEnt->getSkeleton()->setBlendMode(ANIMBLEND_CUMULATIVE);

	String animNames[] =
	{"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
	"SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"};

	// populate our animation list
	for (int i = 0; i < NUM_ANIMS; i++)
	{
		mAnims[i] = mBodyEnt->getAnimationState(animNames[i]);
		mAnims[i]->setLoop(true);
		mFadingIn[i] = false;
		mFadingOut[i] = false;
	}

	// start off in the idle state (top and bottom together)
	setBaseAnimation(ANIM_IDLE_BASE);
	setTopAnimation(ANIM_IDLE_TOP);

	// relax the hands since we're not holding anything
	mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);

	mSwordsDrawn = false;
}

//-------------------------------------------------------------------------------------
void KBEntity::setupCamera(Camera* cam)
{
	mCamera = cam;

	// create a pivot at roughly the character's shoulder
	mCameraPivot = cam->getSceneManager()->getRootSceneNode()->createChildSceneNode();
	// this is where the camera should be soon, and it spins around the pivot
	mCameraGoal = mCameraPivot->createChildSceneNode(Vector3(0, 0, 5));
	// this is where the camera actually is
	mCameraNode = cam->getSceneManager()->getRootSceneNode()->createChildSceneNode();
	mCameraNode->setPosition(mCameraPivot->getPosition() + mCameraGoal->getPosition());

	mCameraPivot->setFixedYawAxis(true);
	mCameraGoal->setFixedYawAxis(true);
	mCameraNode->setFixedYawAxis(true);

	// our model is quite small, so reduce the clipping planes
	mCameraNode->attachObject(cam);

	mPivotPitch = 0;
}

//-------------------------------------------------------------------------------------
void KBEntity::updateBody(Real deltaTime)
{
	mGoalDirection = Vector3::ZERO;   // we will calculate this

	if (mKeyDirection != Vector3::ZERO && mBaseAnimID != ANIM_DANCE)
	{
		// calculate actually goal direction in world based on player's key directions
		mGoalDirection += mKeyDirection.z * mCameraNode->getOrientation().zAxis();
		mGoalDirection += mKeyDirection.x * mCameraNode->getOrientation().xAxis();
		mGoalDirection.y = 0;
		mGoalDirection.normalise();
			
		Quaternion toGoal = mBodyNode->getOrientation().zAxis().getRotationTo(mGoalDirection);
		
		// calculate how much the character has to turn to face goal direction
		Real yawToGoal = toGoal.getYaw().valueDegrees();
		// this is how much the character CAN turn this frame
		Real yawAtSpeed = yawToGoal / Math::Abs(yawToGoal) * deltaTime * (TURN_SPEED * mScale);
		// reduce "turnability" if we're in midair
		if (mBaseAnimID == ANIM_JUMP_LOOP) yawAtSpeed *= 0.2f;

		// turn as much as we can, but not more than we need to
		if (yawToGoal < 0) yawToGoal = std::min<Real>(0, std::max<Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, yawAtSpeed, 0);
		else if (yawToGoal > 0) yawToGoal = std::max<Real>(0, std::min<Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, 0, yawAtSpeed);
		
		mBodyNode->yaw(Degree(yawToGoal));

		// move in current body direction (not the goal direction)
		mBodyNode->translate(0, 0, deltaTime * mMoveSpeed * mAnims[mBaseAnimID]->getWeight(),
			Node::TS_LOCAL);
	}

	if (mBaseAnimID == ANIM_JUMP_LOOP)
	{
		// if we're jumping, add a vertical offset too, and apply gravity
		mBodyNode->translate(0, mVerticalVelocity * deltaTime, 0, Node::TS_LOCAL);
		mVerticalVelocity -= GRAVITY * mScale * deltaTime;
		
		if(_checkJumpEnd())
		{
			setBaseAnimation(ANIM_JUMP_END, true);
			mTimer = 0;
		}
	}
}

//-------------------------------------------------------------------------------------
void KBEntity::updateAnimations(Real deltaTime)
{
	Real baseAnimSpeed = 1;
	Real topAnimSpeed = 1;

	mTimer += deltaTime;

	if (mTopAnimID == ANIM_DRAW_SWORDS)
	{
		// flip the draw swords animation if we need to put it back
		topAnimSpeed = mSwordsDrawn ? -1 : 1;

		// half-way through the animation is when the hand grasps the handles...
		if (mTimer >= mAnims[mTopAnimID]->getLength() / 2 &&
			mTimer - deltaTime < mAnims[mTopAnimID]->getLength() / 2)
		{
			// so transfer the swords from the sheaths to the hands
			mBodyEnt->detachAllObjectsFromBone();
			mBodyEnt->attachObjectToBone(mSwordsDrawn ? "Sheath.L" : "Handle.L", mSword1);
			mBodyEnt->attachObjectToBone(mSwordsDrawn ? "Sheath.R" : "Handle.R", mSword2);
			// change the hand state to grab or let go
			mAnims[ANIM_HANDS_CLOSED]->setEnabled(!mSwordsDrawn);
			mAnims[ANIM_HANDS_RELAXED]->setEnabled(mSwordsDrawn);

			// toggle sword trails
			if (mSwordsDrawn)
			{
				mSwordTrail->setVisible(false);
				mSwordTrail->removeNode(mSword1->getParentNode());
				mSwordTrail->removeNode(mSword2->getParentNode());
				
				if(pWeaponTrailRight_)
				pWeaponTrailRight_->setActive(false);
				if(pWeaponTrailLeft_)
				pWeaponTrailLeft_->setActive(false);
			}
			else
			{
				mSwordTrail->setVisible(true);
				mSwordTrail->addNode(mSword1->getParentNode());
				mSwordTrail->addNode(mSword2->getParentNode());

				if(pWeaponTrailRight_)
				pWeaponTrailRight_->setActive(true);
				if(pWeaponTrailLeft_)
				pWeaponTrailLeft_->setActive(true);
			}
		}

		if (mTimer >= mAnims[mTopAnimID]->getLength())
		{
			// animation is finished, so return to what we were doing before
			if (mBaseAnimID == ANIM_IDLE_BASE) setTopAnimation(ANIM_IDLE_TOP);
			else
			{
				setTopAnimation(ANIM_RUN_TOP);
				mAnims[ANIM_RUN_TOP]->setTimePosition(mAnims[ANIM_RUN_BASE]->getTimePosition());
			}
			mSwordsDrawn = !mSwordsDrawn;
		}
	}
	else if (mTopAnimID == ANIM_SLICE_VERTICAL || mTopAnimID == ANIM_SLICE_HORIZONTAL)
	{
		if (mTimer >= mAnims[mTopAnimID]->getLength())
		{
			// animation is finished, so return to what we were doing before
			if (mBaseAnimID == ANIM_IDLE_BASE) setTopAnimation(ANIM_IDLE_TOP);
			else
			{
				setTopAnimation(ANIM_RUN_TOP);
				mAnims[ANIM_RUN_TOP]->setTimePosition(mAnims[ANIM_RUN_BASE]->getTimePosition());
			}
		}

		// don't sway hips from side to side when slicing. that's just embarrassing.
		if (mBaseAnimID == ANIM_IDLE_BASE) baseAnimSpeed = 0;
	}
	else if (mBaseAnimID == ANIM_JUMP_START)
	{
		if (mTimer >= mAnims[mBaseAnimID]->getLength())
		{
			mIsJump = true;
			// takeoff animation finished, so time to leave the ground!
			setBaseAnimation(ANIM_JUMP_LOOP, true);
			// apply a jump acceleration to the character
			mVerticalVelocity = JUMP_ACCEL * mScale;
		}
	}
	else if (mBaseAnimID == ANIM_JUMP_END)
	{
		if (mTimer >= mAnims[mBaseAnimID]->getLength())
		{
			mIsJump = false;
			// safely landed, so go back to running or idling
			if (mKeyDirection == Vector3::ZERO)
			{
				setBaseAnimation(ANIM_IDLE_BASE);
				setTopAnimation(ANIM_IDLE_TOP);
			}
			else
			{
				setBaseAnimation(ANIM_RUN_BASE, true);
				setTopAnimation(ANIM_RUN_TOP, true);
			}
		}
	}

	// increment the current base and top animation times
	if (mBaseAnimID != ANIM_NONE) mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
	if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

	// apply smooth transitioning between our animations
	fadeAnimations(deltaTime);
}

//-------------------------------------------------------------------------------------
void KBEntity::fadeAnimations(Real deltaTime)
{
	for (int i = 0; i < NUM_ANIMS; i++)
	{
		if (mFadingIn[i])
		{
			// slowly fade this animation in until it has full weight
			Real newWeight = mAnims[i]->getWeight() + deltaTime * (ANIM_FADE_SPEED * mScale);
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight >= 1) mFadingIn[i] = false;
		}
		else if (mFadingOut[i])
		{
			// slowly fade this animation out until it has no weight, and then disable it
			Real newWeight = mAnims[i]->getWeight() - deltaTime * (ANIM_FADE_SPEED * mScale);
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight <= 0)
			{
				mAnims[i]->setEnabled(false);
				mFadingOut[i] = false;
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void KBEntity::updateCamera(Real deltaTime)
{
	// place the camera pivot roughly at the character's shoulder
	mCameraPivot->setPosition(mBodyNode->getPosition() + Vector3::UNIT_Y * (CAM_HEIGHT * mScale));
	// move the camera smoothly to the goal
	Vector3 goalOffset = mCameraGoal->_getDerivedPosition() - mCameraNode->getPosition();
	mCameraNode->translate(goalOffset * deltaTime * 9.0f);
	// always look at the pivot
	mCameraNode->lookAt(mCameraPivot->_getDerivedPosition(), Node::TS_WORLD);
}

//-------------------------------------------------------------------------------------
void KBEntity::updateCameraGoal(Real deltaYaw, Real deltaPitch, Real deltaZoom)
{
	mCameraPivot->yaw(Degree(deltaYaw), Node::TS_WORLD);

	// bound the pitch
	if (!(mPivotPitch + deltaPitch > 25 && deltaPitch > 0) &&
		!(mPivotPitch + deltaPitch < -60 && deltaPitch < 0))
	{
		mCameraPivot->pitch(Degree(deltaPitch), Node::TS_LOCAL);
		mPivotPitch += deltaPitch;
	}
	
	Real dist = mCameraGoal->_getDerivedPosition().distance(mCameraPivot->_getDerivedPosition());
	Real distChange = deltaZoom * dist;

	// bound the zoom
	if (!(dist + distChange < 8 && distChange < 0) &&
		!(dist + distChange > 25 && distChange > 0))
	{
		mCameraGoal->translate(0, 0, distChange, Node::TS_LOCAL);
	}
}

//-------------------------------------------------------------------------------------
void KBEntity::setBaseAnimation(AnimID id, bool reset)
{
	if (mBaseAnimID >= 0 && mBaseAnimID < NUM_ANIMS)
	{
		// if we have an old animation, fade it out
		mFadingIn[mBaseAnimID] = false;
		mFadingOut[mBaseAnimID] = true;
	}

	mBaseAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

//-------------------------------------------------------------------------------------
void KBEntity::setTopAnimation(AnimID id, bool reset)
{
	if (mTopAnimID >= 0 && mTopAnimID < NUM_ANIMS)
	{
		// if we have an old animation, fade it out
		mFadingIn[mTopAnimID] = false;
		mFadingOut[mTopAnimID] = true;
	}

	mTopAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

//-------------------------------------------------------------------------------------
void KBEntity::attack(KBEntity* receiver, uint32 skillID, uint32 damageType, uint32 damage)
{
	setTopAnimation(damage % 2 > 0 ? ANIM_SLICE_HORIZONTAL : ANIM_SLICE_VERTICAL, true);
	mTimer = 0;
}

//-------------------------------------------------------------------------------------
void KBEntity::recvDamage(KBEntity* attacker, uint32 skillID, uint32 damageType, uint32 damage)
{
	damageShowTime_ = 3.0f;
	
	if(pDamageLabel_ == NULL)
	{
		pDamageLabel_ = new Ogre::MovableText(Ogre::StringConverter::toString(mID) + "_damage", damage > 0 ? "-" + Ogre::StringConverter::toString(damage) : "Miss", 
			"Yahei", 1.0f, Ogre::ColourValue::Black);

		pDamageLabel_->setTextAlignment(Ogre::MovableText::H_CENTER, Ogre::MovableText::V_ABOVE); 
		pDamageLabel_->showOnTop(true);
		pDamageLabel_->setColor(Ogre::ColourValue::Red);
		pDamageLabelNode_->attachObject(pDamageLabel_);
	}
	else
	{
		pDamageLabel_->setCaption(damage > 0 ? "-" + Ogre::StringConverter::toString(damage) : "Miss");
	}

	hp_ -= damage;
	
	if(hp_max_ <= 0)
		return;

	// 改变头顶血量显示
	Billboard* health = pHealthHUD_->getBillboard(0);
	float healthPer = hp_ / float(hp_max_);
	float healthLength = healthPer * pHealthHUD_->getDefaultWidth();
	health->setDimensions(healthLength, pHealthHUD_->getDefaultHeight());
	ColourValue maxHealthCol = ColourValue::Blue;
	ColourValue minHealthCol = ColourValue::Red;
	ColourValue currHealthCol = maxHealthCol * healthPer + minHealthCol * (1 - healthPer);
	health->setColour(currHealthCol);
}

//-------------------------------------------------------------------------------------
void KBEntity::setHighlighted( bool highlight )
{
	Ogre::String nameExtension = "_HighLight";
    Ogre::Entity* ent = mBodyEnt;
    int numEnts = ent->getNumSubEntities();

    for( int i = 0; i < numEnts; i++ )
    {
        Ogre::SubEntity* subent = ent->getSubEntity(i);

        if( subent == NULL )
            continue;

        // TODO - optimieren, nur wenn der Typ veraendert wird
        //if(StringUtil::endsWith(subent->getMaterialName(),nameExtension)
        // == highlight )
        //  continue;

        if (ent->isHardwareAnimationEnabled())
        {
            subent->setCustomParameter(0,
                highlight ? Vector4(1, 1, 1, 1) : Vector4(0, 0, 0, 0));
        }

        Ogre::MaterialPtr oldMaterial = subent->getMaterial();

        // Highlight setzen
        if( highlight )
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName( oldMaterial->getName()+nameExtension );

            if( material.isNull() )
            {
                material = oldMaterial->clone( oldMaterial->getName()+nameExtension );

                material->setAmbient(1.0f, 1.0f, 1.0f);
                material->setDiffuse(1.0f, 1.0f, 1.0f, 1.0f);
                material->setSelfIllumination(0.4f, 0.4f, 0.4f);
            }

            subent->setMaterialName(material->getName());
        }
        // Highlight entfernen
        else
        {
            Ogre::String matName = oldMaterial->getName();
            matName = matName.erase(matName.length() - nameExtension.length(), nameExtension.length() );
            subent->setMaterialName( matName );
        }
    }
}

//-------------------------------------------------------------------------------------