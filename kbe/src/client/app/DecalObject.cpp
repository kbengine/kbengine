#include "space_world.h"
#include "DecalObject.h"

//---------------------------------------------------------------------------//
DecalObject::DecalObject(Ogre::String  decalMaterialName, Space* pSpace, float radiusMin, float radiusMax):
pSpace_(pSpace),
pDecal_(NULL),
decalSize_(1.f),
decalSizeMin_(radiusMin),
decalSizeMax_(radiusMax),
decalSizeInc_(false),
pos_()
{
	if(decalMaterialName.size() > 0)
		createDecal(decalMaterialName);
}

//---------------------------------------------------------------------------//
DecalObject::~DecalObject()
{
}

//----------------------------------------------------------------------------------------
Ogre::ManualObject* DecalObject::createDecal(Ogre::String decalMaterialName)
{
	pDecal_ = new Ogre::ManualObject(decalMaterialName);
    pSpace_->pSceneMgr()->getRootSceneNode()->createChildSceneNode()->attachObject(pDecal_);
   // pDecal_->setCastShadows(false);
	//pDecal_->setDynamic(true);  
	//pDecal_->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_2);  

	int x_size = 64;  // number of polygons
	int z_size = 64;
    pDecal_->begin(decalMaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);

    for (int i=0; i<x_size; i++)
    {
        for (int j=0; j<=z_size; j++)
        {
            pDecal_->position(Ogre::Vector3(i, 0, j));
            pDecal_->textureCoord((float)i / (float)x_size, (float)j / (float)z_size);
        }
    }

    for (int i=0; i<x_size; i++)
    {
        for (int j=0; j<z_size; j++)
        {
            pDecal_->quad( i * (x_size+1) + j, i * (x_size+1) + j + 1,
                (i + 1) * (x_size+1) + j + 1,(i + 1) * (x_size+1) + j);
        }
    }

    pDecal_->end();

	return pDecal_;
}

//----------------------------------------------------------------------------------------
void DecalObject::moveDecalTo(const Ogre::Vector3& position)
{
	pos_ = position;
	decalSize_ = decalSizeMax_;
	decalSizeInc_ = false;
}

//---------------------------------------------------------------------------//
void DecalObject::update(Ogre::Real delta)
{
	if(decalSizeInc_)
	{
		decalSize_ += 0.5f * delta;

		if(decalSize_ > decalSizeMax_)
			decalSizeInc_ = false;
	}
	else
	{
		decalSize_ -= 0.5f * delta;
		if(decalSize_ <= decalSizeMin_)
			decalSizeInc_ = true;
	}

	Ogre::Real x = pos_.x;
	Ogre::Real z = pos_.z;

    Ogre::Real x1 = x - decalSize_;
    Ogre::Real z1 = z - decalSize_;

	int x_size = 64;  // number of polygons
	int z_size = 64;

    Ogre::Real x_step = decalSize_ * 2/ x_size;
    Ogre::Real z_step = decalSize_ * 2/ z_size;

    pDecal_->beginUpdate(0);

    for (int i=0; i<=x_size; i++)
    {
        for (int j=0; j<=z_size; j++)
        {    
			pDecal_->position(Ogre::Vector3(x1, static_cast<SpaceWorld*>(pSpace_)->getPositionHeight(Ogre::Vector3(x1, 0, z1)) + 0.1, z1));
            pDecal_->textureCoord((float)i / (float)x_size, (float)j / (float)z_size);

            z1 += z_step;
        }

        x1 += x_step;

        z1 = z - decalSize_;
    }

    for (int i=0; i<x_size; i++)
    {
        for (int j=0; j<z_size; j++)
        {
            pDecal_->quad( i * (x_size+1) + j, i * (x_size+1) + j + 1, 
                (i + 1) * (x_size+1) + j + 1, (i + 1) * (x_size+1) + j);
        }
    }

    pDecal_->end();
}

//---------------------------------------------------------------------------//

