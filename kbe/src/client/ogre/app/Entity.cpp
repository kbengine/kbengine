#include "Entity.h"
#include "WeaponTrail.h"
#include "space_world.h"
#include "DotSceneLoader.h"
#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include "../../kbengine_dll/kbengine_dll.h"

//-------------------------------------------------------------------------------------
KBEntity::KBEntity(SpaceWorld* pSpace, KBEngine::ENTITY_ID eid):
  mCamera(NULL),
  mBodyNode(NULL),
  mBodyEnt(NULL),
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
  char_height_(4.f),
  modelName_(),
  modelID_(0)
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

	if(pNameLabelNode_)mSceneMgr->destroySceneNode(pNameLabelNode_);
	if(pDamageLabelNode_)mSceneMgr->destroySceneNode(pDamageLabelNode_);
	if(pHudNode_)mSceneMgr->destroySceneNode(pHudNode_);

	if(mBodyEnt)mSceneMgr->destroyEntity(mBodyEnt);

	if(mBodyNode)mSceneMgr->destroySceneNode(mBodyNode);
}

//-------------------------------------------------------------------------------------
void KBEntity::addTime(Real deltaTime)
{
	stickto(deltaTime);

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
		Ogre::Real distanceAboveTerrain = calcDistanceAboveTerrain();

		if (camPos.y <= rayResult.position.y + distanceAboveTerrain)
		{
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
void KBEntity::stickto(Real deltaTime)
{
	if(!isJump())
	{
		Ogre::Vector3 currpos = getPosition();

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

		setPosition(currpos.x, currpos.y, currpos.z);
	}
}

//-------------------------------------------------------------------------------------
void KBEntity::setup(SceneManager* sceneMgr)
{
	setupBody(sceneMgr);
	setupAnimations();
}

//-------------------------------------------------------------------------------------
void KBEntity::setupBody(SceneManager* sceneMgr)
{
	mSceneMgr = sceneMgr;

	// create main model
	mBodyNode = sceneMgr->getRootSceneNode()->createChildSceneNode();

	Ogre::String sKey = Ogre::StringConverter::toString(mID);
	mBodyEnt = sceneMgr->createEntity(sKey + "_Body", modelName_);
	mBodyEnt->setQueryFlags(Space::ENTITY_MASK);
	mBodyNode->attachObject(mBodyEnt);
	
	//mBodyNode->scale(mScale, mScale, mScale);
	char_height_ = mBodyEnt->getWorldBoundingBox(true).getSize().y * mScale;

	mKeyDirection = Vector3::ZERO;
	mVerticalVelocity = 0;

	// 绑定名称
	pNameLabelNode_ = sceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::StringConverter::toString(mID) + "_name");
//	pNameLabelNode_->setScale(0.5f, 0.5f, 0.5f);
	pNameLabel_ = new Ogre::MovableText(Ogre::StringConverter::toString(mID) + "_name", mName, 
		"Yahei", 0.5f, Ogre::ColourValue::Black);

	pNameLabel_->setTextAlignment(Ogre::MovableText::H_CENTER, Ogre::MovableText::V_CENTER); 
	pNameLabel_->showOnTop(true);
	pNameLabel_->setColor(Ogre::ColourValue::Blue);

	pNameLabelNode_->attachObject(pNameLabel_);
	pNameLabelNode_->setInheritScale(false);
	
	pDamageLabelNode_ = sceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::StringConverter::toString(mID) + "_damage");

	// 创建头顶血条
	pHealthHUD_ = sceneMgr->createBillboardSet();
	pHealthHUD_->setMaterialName("Examples/Billboard");
	pHealthHUD_->setDefaultWidth(1);
	pHealthHUD_->setDefaultHeight(0.05f);

	pHudNode_ = sceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::StringConverter::toString(mID) + "_hud");
	pHudNode_->setInheritScale(false);
	pHudNode_->attachObject(pHealthHUD_);

	Billboard* b = pHealthHUD_->createBillboard(0, 0, 0);
	b->setColour(Ogre::ColourValue::Green);
	//b->setDimensions(96, 12);
	b = NULL;
	//scale(mScale);
}

//-------------------------------------------------------------------------------------
void KBEntity::injectKeyDown(const OIS::KeyEvent& evt)
{
}

//-------------------------------------------------------------------------------------
void KBEntity::injectKeyUp(const OIS::KeyEvent& evt)
{
}

#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS) || (OGRE_PLATFORM == OGRE_PLATFORM_ANDROID)
//-------------------------------------------------------------------------------------
void KBEntity::injectMouseMove(const OIS::MultiTouchEvent& evt)
{
}

//-------------------------------------------------------------------------------------
void KBEntity::injectMouseDown(const OIS::MultiTouchEvent& evt)
{
}
#else
//-------------------------------------------------------------------------------------
void KBEntity::injectMouseMove(const OIS::MouseEvent& evt)
{
}

//-------------------------------------------------------------------------------------
void KBEntity::injectMouseDown(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
}
#endif

//-------------------------------------------------------------------------------------
void KBEntity::setupAnimations()
{
}

//-------------------------------------------------------------------------------------
void KBEntity::attack(KBEntity* receiver, uint32 skillID, uint32 damageType, uint32 damage)
{
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
		pDamageLabel_->setCaption(damage > 0 ? "-" + Ogre::StringConverter::toString(damage) : "MISS");
	}

	hp_ -= damage;
	updateHPHud();
}

//-------------------------------------------------------------------------------------
void KBEntity::updateHPHud()
{
	if(hp_max_ <= 0)
		return;

	// 改变头顶血量显示
	Billboard* health = pHealthHUD_->getBillboard(0);
	float healthPer = hp_ / float(hp_max_);

	float healthLength = healthPer * pHealthHUD_->getDefaultWidth();
	if(healthLength < 0.0004f)
		healthLength = 0.f;

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