#include "EntityComplex.h"
#include "WeaponTrail.h"
#include "space_world.h"
#include "DotSceneLoader.h"
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include "../kbengine_dll/kbengine_dll.h"

//-------------------------------------------------------------------------------------
EntityComplex::EntityComplex(SpaceWorld* pSpace, KBEngine::ENTITY_ID eid):
	KBEntity(pSpace, eid),
  mCameraPivot(NULL),
  mCameraGoal(NULL),
  mCameraNode(NULL),
  mPivotPitch(0.0f),
  mSword1(NULL),
  mSword2(NULL),
  mSwordTrail(NULL),
  pWeaponTrailLeft_(NULL),
  pWeaponTrailRight_(NULL)
{
	//setupBody(cam->getSceneManager());
	//setupCamera(cam);
	//setupAnimations();

	modelName_ = "Sinbad.mesh";
}

//-------------------------------------------------------------------------------------
EntityComplex::~EntityComplex()
{
	SAFE_RELEASE(pWeaponTrailLeft_);
	SAFE_RELEASE(pWeaponTrailRight_);

	if(mSword1)mSceneMgr->destroyEntity(mSword1);
	if(mSword2)mSceneMgr->destroyEntity(mSword2);

	if(mSwordTrail)mSceneMgr->destroyMovableObject(mSwordTrail);

	if(mCameraPivot)mSceneMgr->destroySceneNode(mCameraPivot);
	if(mCameraGoal)mSceneMgr->destroySceneNode(mCameraGoal);
	if(mCameraNode)mSceneMgr->destroySceneNode(mCameraNode);
}

//-------------------------------------------------------------------------------------
void EntityComplex::addTime(Real deltaTime)
{
	if(mBodyNode)
	{
		if(mCamera)
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

	setPosition(currpos.x, currpos.y, currpos.z);
	KBEntity::addTime(deltaTime);

	if(pWeaponTrailLeft_)
		pWeaponTrailLeft_->onUpdate(deltaTime);

	if(pWeaponTrailRight_)
		pWeaponTrailRight_->onUpdate(deltaTime);
}

//-------------------------------------------------------------------------------------
void EntityComplex::setupBody(SceneManager* sceneMgr)
{
	KBEntity::setupBody(sceneMgr);
	Ogre::String sKey = Ogre::StringConverter::toString(mID);

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
}

//-------------------------------------------------------------------------------------
void EntityComplex::injectKeyDown(const OIS::KeyEvent& evt)
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
void EntityComplex::injectKeyUp(const OIS::KeyEvent& evt)
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
void EntityComplex::injectMouseMove(const OIS::MultiTouchEvent& evt)
{
	// update camera goal based on mouse movement
	updateCameraGoal(-0.05f * evt.state.X.rel, -0.05f * evt.state.Y.rel, -0.1f * evt.state.Z.rel);
}

//-------------------------------------------------------------------------------------
void EntityComplex::injectMouseDown(const OIS::MultiTouchEvent& evt)
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
void EntityComplex::injectMouseMove(const OIS::MouseEvent& evt)
{
	// update camera goal based on mouse movement
	if (evt.state.buttonDown(OIS::MB_Right)) //按下右键后旋转
	{
		updateCameraGoal(-0.1f * evt.state.X.rel, -0.1f * evt.state.Y.rel, -0.1f * evt.state.Z.rel);
	}
}

//-------------------------------------------------------------------------------------
void EntityComplex::injectMouseDown(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
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
void EntityComplex::setupAnimations()
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
void EntityComplex::setupCamera(Camera* cam)
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
void EntityComplex::updateBody(Real deltaTime)
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
void EntityComplex::updateAnimations(Real deltaTime)
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
void EntityComplex::fadeAnimations(Real deltaTime)
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
void EntityComplex::updateCamera(Real deltaTime)
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
void EntityComplex::updateCameraGoal(Real deltaYaw, Real deltaPitch, Real deltaZoom)
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
void EntityComplex::setBaseAnimation(AnimID id, bool reset)
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
void EntityComplex::setTopAnimation(AnimID id, bool reset)
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
void EntityComplex::attack(KBEntity* receiver, uint32 skillID, uint32 damageType, uint32 damage)
{
	setTopAnimation(damage % 2 > 0 ? ANIM_SLICE_HORIZONTAL : ANIM_SLICE_VERTICAL, true);
	mTimer = 0;
}

//-------------------------------------------------------------------------------------