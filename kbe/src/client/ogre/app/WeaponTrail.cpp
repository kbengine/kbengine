#include "WeaponTrail.h"

//---------------------------------------------------------------------------//
WeaponTrail::WeaponTrail(Ogre::String  name, Ogre::SceneManager* s )
:m_TrailObject(0),
m_MaxSegmentCount(30),
mSceneMgr(s),
m_IsActive(true),
mName(name)
{
	m_SegmentStartColorChange = Ogre::ColourValue(1.0f,1.0f,1.0f,1.0f);
	m_SegmentEndColorChange = Ogre::ColourValue(1.0f,1.0f,1.0f,1.0f);
	m_SegmentStartInitialColor = Ogre::ColourValue(0.6f,0.5f,0.8f,1.0f);
	m_SegmentEndInitialColor = Ogre::ColourValue(1.0f,0.2f,1.0f,1.0f);

	m_SegmentStartColorChange *= 3.0f;
	m_SegmentEndColorChange *= 3.0f;

	m_Width = 30.0f;

	setWeaponEntity(0);
	init();
}
//---------------------------------------------------------------------------//
WeaponTrail::~WeaponTrail()
{
	uninit();
}
//---------------------------------------------------------------------------//
void WeaponTrail::init()
{
	// create object
	m_TrailObject =
		mSceneMgr->createManualObject(mName);
	m_TrailObject->estimateVertexCount(m_MaxSegmentCount * 2);
	m_TrailObject->setDynamic(true);

	m_TrailObject->begin("sword/AxeTrail", Ogre::RenderOperation::OT_TRIANGLE_STRIP);

	// fill the object (the actual data does not matter here)
	for(int i=0; i<m_MaxSegmentCount; ++i)
	{
		m_TrailObject->position(0.f, 0.f, -i*20.f);
		m_TrailObject->textureCoord(0.f,0.f);
		m_TrailObject->colour(1.f,0.f,0.f,1.f);
		m_TrailObject->position(0.f, 30.f, -i*20.f);
		m_TrailObject->textureCoord(1.f,0.f);
		m_TrailObject->colour(1.f,0.f,0.f,1.f);
	}
	m_TrailObject->end();

	// create node and attach object
	m_TrailNode =
		mSceneMgr->getRootSceneNode()->createChildSceneNode();
	m_TrailNode->attachObject(m_TrailObject);
	m_TrailObject->setVisible(1);
}
//---------------------------------------------------------------------------//
void WeaponTrail::uninit()
{
	m_IsActive = false;

	// detach object and remove node
	m_TrailNode->detachObject(m_TrailObject);
	mSceneMgr->getRootSceneNode()->
		removeAndDestroyChild(m_TrailNode->getName());

	// remove object
	m_TrailObject->setVisible(false);
	mSceneMgr->destroyManualObject(m_TrailObject); 
}
//---------------------------------------------------------------------------//
void WeaponTrail::setWeaponEntity(Ogre::Entity* p_WeaponEntity)
{
	m_WeaponEntity = p_WeaponEntity;
	if (m_WeaponEntity)
	{   
		m_WeaponNode = m_WeaponEntity->getParentNode();
	}
	else
	{
		m_WeaponNode = 0;
	}
}
//---------------------------------------------------------------------------//
void WeaponTrail::onUpdate(float p_DeltaT)
{
	// early out
	if(!isActive() && !isVisible())
	{
		return;
	}
	if (!m_WeaponEntity || !m_WeaponNode)
	{
		return;
	}
	if (!m_TrailObject)
	{
		return;
	}

	m_TrailObject->setVisible(true);

	// iterate over the current segments, apply alpha change
	for(TrailSegmentList::iterator it = m_SegmentList.begin();
		it != m_SegmentList.end();)
	{
		(*it).segmentStartColor -= m_SegmentStartColorChange * p_DeltaT;
		(*it).segmentEndColor -= m_SegmentEndColorChange * p_DeltaT;
		(*it).segmentStartColor.saturate();
		(*it).segmentEndColor.saturate();
		if((*it).segmentStartColor == Ogre::ColourValue::ZERO && (*it).segmentEndColor == Ogre::ColourValue::ZERO)
		{
			it = m_SegmentList.erase(it);
		}
		else
		{
			++it;
		}
	}

	// throw away the last element if the maximum number of segments is used
	if((int)m_SegmentList.size() >= m_MaxSegmentCount)
	{
		m_SegmentList.pop_back();
	}

	// only add a new segment if active
	if(isActive())
	{
		// the segment to add to the trail
		TrailSegment newSegment;
		// initial the trail
		newSegment.segmentStartColor = getSegmentStartInitialColor();
		newSegment.segmentEndColor = getSegmentEndInitialColor();
		newSegment.segmentStart  = m_WeaponNode->_getDerivedPosition();
		Ogre::Vector3 pos = m_WeaponNode->getPosition();
		// probably quite costly way to get the second position
		m_WeaponNode->translate(Ogre::Vector3(0, m_Width, 0), Ogre::SceneNode::TS_LOCAL);
		newSegment.segmentEnd = m_WeaponNode->_getDerivedPosition();
		m_WeaponNode->setPosition(pos);

		Ogre::Vector3 _verDir = newSegment.segmentEnd - newSegment.segmentStart;
		_verDir.normalise();

		newSegment.segmentEnd = newSegment.segmentStart + _verDir *  m_Width;

		m_SegmentList.push_front(newSegment);

	}
	// update the manual object
	m_TrailObject->beginUpdate(0);
	int segmentCount = 0;
	for(TrailSegmentList::iterator it = m_SegmentList.begin();
		it != m_SegmentList.end(); ++it)
	{
		m_TrailObject->position((*it).segmentStart);
		m_TrailObject->textureCoord(0,0);
		m_TrailObject->colour((*it).segmentStartColor);
		m_TrailObject->position((*it).segmentEnd);
		m_TrailObject->textureCoord(1,0);
		m_TrailObject->colour((*it).segmentEndColor);
		++segmentCount;
	}
	// use the last position to render the invisible part of the trail
	// as degenerate triangles
	Ogre::Vector3 lastPos = Ogre::Vector3::ZERO;
	if(!m_SegmentList.empty())
	{
		lastPos = m_SegmentList.back().segmentStart;
	}
	for(int i=segmentCount*2;i<m_MaxSegmentCount * 2;++i)
	{
		m_TrailObject->position(lastPos);
	}
	// end the update
	m_TrailObject->end();
}
//---------------------------------------------------------------------------//
void WeaponTrail::setMaterialName(const Ogre::String& p_MaterialName)
{
	m_MaterialName = p_MaterialName;
	if(m_TrailObject)
	{
		m_TrailObject->setMaterialName(0, m_MaterialName);
	}
}
//---------------------------------------------------------------------------//
void WeaponTrail::setSegmentStartColorChange(const Ogre::ColourValue& p_ColorChange)
{
	m_SegmentStartColorChange = p_ColorChange;
}
//---------------------------------------------------------------------------//
const Ogre::ColourValue& WeaponTrail::getSegmentStartColorChange() const
{
	return m_SegmentStartColorChange;
}
//---------------------------------------------------------------------------//
void WeaponTrail::setSegmentEndColorChange(const Ogre::ColourValue& p_ColorChange)
{
	m_SegmentEndColorChange = p_ColorChange;
}
//---------------------------------------------------------------------------//
const Ogre::ColourValue& WeaponTrail::getSegmentEndColorChange() const
{
	return m_SegmentEndColorChange;
}
//---------------------------------------------------------------------------//
void WeaponTrail::setSegmentStartInitialColor(const Ogre::ColourValue& p_Color)
{
	m_SegmentStartInitialColor = p_Color;
}
//---------------------------------------------------------------------------//
const Ogre::ColourValue& WeaponTrail::getSegmentStartInitialColor() const
{
	return m_SegmentStartInitialColor;
}
//---------------------------------------------------------------------------//
void WeaponTrail::setSegmentEndInitialColor(const Ogre::ColourValue& p_Color)
{
	m_SegmentEndInitialColor = p_Color;
}
//---------------------------------------------------------------------------//
const Ogre::ColourValue& WeaponTrail::getSegmentEndInitialColor() const
{
	return m_SegmentEndInitialColor;
}
//---------------------------------------------------------------------------//
void WeaponTrail::setActive(bool p_Active)
{
	m_IsActive = p_Active;
}
//---------------------------------------------------------------------------//
bool WeaponTrail::isActive() const
{
	return m_IsActive;
}
//---------------------------------------------------------------------------//
bool WeaponTrail::isVisible() const
{
	return !m_SegmentList.empty();
}
//---------------------------------------------------------------------------//

