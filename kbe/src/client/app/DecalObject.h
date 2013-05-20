#pragma once
#include "Ogre.h"
#include<list>

class Space;

class DecalObject
{
public:

	DecalObject(Ogre::String  decalMaterialName, Space* pSpace, float radiusMin = 1.f, float radiusMax = 1.f);

	virtual ~DecalObject();

	void moveDecalTo(const Ogre::Vector3& position);
	Ogre::ManualObject* createDecal(Ogre::String decalMaterialName);
	
	void update(Ogre::Real delta);
protected:
	Space* pSpace_;
	Ogre::ManualObject* pDecal_;

	float decalSize_, decalSizeMin_, decalSizeMax_;
	bool decalSizeInc_;

	Ogre::Vector3 pos_;

}; // end of WeaponTrail class declaration

